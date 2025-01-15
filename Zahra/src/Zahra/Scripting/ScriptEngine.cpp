#include "zpch.h"
#include "ScriptEngine.h"

#include "Zahra/Scene/Scene.h"
#include "Zahra/Scripting/ScriptGlue.h"
#include "Zahra/Utils/FileIO.h"

#include "FileWatch.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/tabledefs.h>

namespace Zahra
{
	namespace MonoUtils
	{
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			Z_CORE_ASSERT(std::filesystem::exists(assemblyPath), "Assembly file does not exist");

			uint32_t fileSize = 0;
			char* fileData = Zahra::FileIO::ReadBytes(assemblyPath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				Z_CORE_ERROR("Failed to load MonoImage: {0}", errorMessage);
				return nullptr;
			}

			std::string assemblyPathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPathString.c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

			return assembly;
		}
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		// TODO: get these from a ProjectConfig (when that exists...)
		const std::filesystem::path ProjectDirectory = "../Examples/Bud";
		const std::string ProjectScriptLibrary = "Bud.dll";

		const std::filesystem::path ScriptLibraryDirectory = ProjectDirectory / "Assets/Scripts/Binaries";
		const std::filesystem::path CoreAssemblyFilepath = ScriptLibraryDirectory / "Djinn.dll";
		const std::filesystem::path AppAssemblyFilepath = ScriptLibraryDirectory / ProjectScriptLibrary;

		// TODO: add logic to disable this filewatcher
		// (likely unnecessary overhead in a shipped game e.g.)
		Scope<filewatch::FileWatch<std::string>> AssemblyFileWatcher;
		bool AssemblyReloadPending = false;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;
		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;
		std::unordered_map<ZGUID, Ref<ScriptInstance>> ScriptInstances;

		Scene* SceneContext = nullptr;

		const std::unordered_map<std::string, ScriptFieldType> MonoTypeNameToScriptFieldType = 
		{
			{ "System.Boolean",		ScriptFieldType::Bool    },
			{ "System.SByte",		ScriptFieldType::sByte   },
			{ "System.Byte",		ScriptFieldType::Byte    },
			{ "System.Int16",		ScriptFieldType::Short   },
			{ "System.UInt16",		ScriptFieldType::uShort  },
			{ "System.Char",		ScriptFieldType::Char    },
			{ "System.Int32",		ScriptFieldType::Int     },
			{ "System.UInt32",		ScriptFieldType::uInt    },
			{ "System.Int64",		ScriptFieldType::Long    },
			{ "System.UInt64",		ScriptFieldType::uLong   },
			{ "System.Single",		ScriptFieldType::Float   },
			{ "System.Double",		ScriptFieldType::Double  },
			{ "Djinn.Vector2",		ScriptFieldType::Vector2 },
			{ "Djinn.Vector3",		ScriptFieldType::Vector3 },
			{ "Djinn.Vector4",		ScriptFieldType::Vector4 },
		};
	};

	static ScriptEngineData* s_SEData;


	void ScriptEngine::Init()
	{
		s_SEData = znew ScriptEngineData();

		CreateRootDomain();
		CreateAppDomain();
		LoadAssemblies();
		InitAssemblyFileWatcher();
		Reflect();

		ScriptGlue::RegisterComponentTypes(s_SEData->CoreAssemblyImage);
		ScriptGlue::RegisterFunctions();

		Z_CORE_INFO("Script engine has initialised");
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMonoDomains();

		delete s_SEData;

		Z_CORE_INFO("Script engine has shut down");
	}

	// TODO: once we have intrusive reference counting, we can pass the scene as a Ref instead of a raw pointer
	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_SEData->AssemblyFileWatcher.reset();

		s_SEData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_SEData->SceneContext = nullptr;
		s_SEData->ScriptInstances.clear();
		//mono_gc_collect(0); // TODO: maybe trigger garbage collection here

		InitAssemblyFileWatcher();
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_SEData->AppDomain);

		CreateAppDomain();
		LoadAssemblies();
		Reflect();

		ScriptGlue::RegisterComponentTypes(s_SEData->CoreAssemblyImage);

		s_SEData->AssemblyReloadPending = false;

		Z_CORE_INFO("Script engine reloaded");
	}

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetScriptClasses()
	{
		return s_SEData->ScriptClasses;
	}

	const Ref<ScriptClass> ScriptEngine::GetScriptClassIfValid(const std::string& fullName)
	{
		auto it = s_SEData->ScriptClasses.find(fullName);

		if (it == s_SEData->ScriptClasses.end())
			return nullptr;
		else
			return it->second;
	}

	bool ScriptEngine::ValidScriptClass(const std::string& fullName)
	{
		return s_SEData->ScriptClasses.find(fullName) != s_SEData->ScriptClasses.end();
	}

	void ScriptEngine::CreateScriptInstance(Entity entity)
	{
		Z_CORE_ASSERT(entity.HasComponents<ScriptComponent>())
		auto& component = entity.GetComponents<ScriptComponent>();

		auto it = s_SEData->ScriptClasses.find(component.ScriptName);
		if (it != s_SEData->ScriptClasses.end())
		{
			auto& scriptClass = it->second;
			Ref<ScriptInstance> instance = Ref<ScriptInstance>::Create(scriptClass, entity.GetGUID());
			s_SEData->ScriptInstances[entity.GetGUID()] = instance;

			auto fields = scriptClass->GetPublicFields();
			auto buffer = s_SEData->SceneContext->GetScriptFieldStorage(entity);

			for (uint64_t i = 0; i < fields.size(); i++)
			{
				auto field = fields[i];
				uint64_t offset = 16 * i;

				// TODO: currently this only works for fields of value-type: simple types,
				// and structs whose fields are also value types. To handle a class e.g. I'd
				// need to take into account the header section of its memory layout.
				switch (ScriptUtils::GetScriptFieldTypeByteSize(field.Type))
				{
					case 1:
					{
						auto value = buffer.ReadAs<uint8_t>(offset);
						instance->SetScriptFieldValue<uint8_t>(field, value);
						break;
					}
					case 2:
					{
						auto value = buffer.ReadAs<uint16_t>(offset);
						instance->SetScriptFieldValue<uint16_t>(field, value);
						break;
					}
					case 4:
					{
						auto value = buffer.ReadAs<uint32_t>(offset);
						instance->SetScriptFieldValue<uint32_t>(field, value);
						break;
					}
					case 8:
					{
						auto value = buffer.ReadAs<uint64_t>(offset);
						instance->SetScriptFieldValue<uint64_t>(field, value);
						break;
					}
					case 16:
					{
						auto value = buffer.ReadAs<glm::vec4>(offset);
						instance->SetScriptFieldValue<glm::vec4>(field, value);
						break;
					}
				}
				
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::ScriptInstanceEarlyUpdate(Entity entity, float dt)
	{
		if (auto instance = GetScriptInstance(entity))
			instance->InvokeEarlyUpdate(dt);
	}

	void ScriptEngine::ScriptInstanceLateUpdate(Entity entity, float dt)
	{
		if (auto instance = GetScriptInstance(entity))
			instance->InvokeLateUpdate(dt);
	}

	Entity ScriptEngine::GetEntity(ZGUID guid)
	{
		Z_CORE_ASSERT(s_SEData->SceneContext);

		return s_SEData->SceneContext->GetEntity(guid);
	}

	Entity ScriptEngine::GetEntity(MonoString* name)
	{
		Z_CORE_ASSERT(s_SEData->SceneContext);

		char* string = mono_string_to_utf8(name);
		Entity entity = s_SEData->SceneContext->GetEntity(string);
		mono_free(string);

		return entity;
	}

	MonoObject* ScriptEngine::GetMonoObject(ZGUID guid)
	{
		auto it = s_SEData->ScriptInstances.find(guid);
		if (it != s_SEData->ScriptInstances.end())
			return it->second->GetMonoObject();

		return nullptr;
	}

	MonoString* ScriptEngine::StdStringToMonoString(const std::string& string)
	{
		MonoString* monoString = mono_string_new(s_SEData->AppDomain, string.c_str());
		return monoString;
	}
		
	void ScriptEngine::CreateRootDomain()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("ZahraJITRuntime");
		Z_CORE_ASSERT(rootDomain);

		s_SEData->RootDomain = rootDomain;		
	}

	void ScriptEngine::CreateAppDomain()
	{
		s_SEData->AppDomain = mono_domain_create_appdomain("DjinnRuntime", nullptr);
		mono_domain_set(s_SEData->AppDomain, true);
	}

	void ScriptEngine::LoadAssemblies()
	{
		LoadAssembly(s_SEData->CoreAssemblyFilepath, s_SEData->CoreAssembly, s_SEData->CoreAssemblyImage);
		LoadAssembly(s_SEData->AppAssemblyFilepath, s_SEData->AppAssembly, s_SEData->AppAssemblyImage);
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& library, MonoAssembly*& assembly, MonoImage*& assemblyImage)
	{
		assembly = MonoUtils::LoadMonoAssembly(library);
		assemblyImage = mono_assembly_get_image(assembly);

		// TODO: add assembly validation
	}

	static void OnAppAssemblyFileWatcherEvent(const std::string& filepath, const filewatch::Event event)
	{
		Z_CORE_WARN("ScriptEngine::AssemblyFileWatcher reported event ('{}', '{}')", filepath.c_str(), filewatch::event_to_string(event));

		if (!s_SEData->AssemblyReloadPending && event == filewatch::Event::modified)
		{
			// this extra logic 'debounces' this callback, as these events typically occur in rapid-fire
			// batches whenever the assembly gets rebuilt. We only want a single assembly reload in response
			// to this, so only the first event in the sequence will trigger it
			s_SEData->AssemblyReloadPending = true;

			// assemblies will reload at the top of the next frame
			Application::Get().SubmitToMainThread([]()
				{
					ScriptEngine::ReloadAssembly();
				});
		}
	}

	void ScriptEngine::InitAssemblyFileWatcher()
	{
		s_SEData->AssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(s_SEData->AppAssemblyFilepath.string(), OnAppAssemblyFileWatcherEvent);
	}

	void ScriptEngine::ShutdownMonoDomains()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_SEData->AppDomain);
		s_SEData->AppDomain = nullptr;

		mono_jit_cleanup(s_SEData->RootDomain);
		s_SEData->RootDomain = nullptr;
	}

	void ScriptEngine::Reflect()
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// FIND ALL ENTITY TYPES
		s_SEData->ScriptClasses.clear();
		ScriptClass entityClass(s_SEData->CoreAssemblyImage, "Djinn", "Entity");

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_SEData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t typeCount = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t row = 0; row < typeCount; row++)
		{
			uint32_t classData[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, row, classData, MONO_TYPEDEF_SIZE);

			// table stores opaque IDs in place of string data
			std::string reflectedNamespace = mono_metadata_string_heap(s_SEData->AppAssemblyImage, classData[MONO_TYPEDEF_NAMESPACE]);
			std::string reflectedName = mono_metadata_string_heap(s_SEData->AppAssemblyImage, classData[MONO_TYPEDEF_NAME]);
			ScriptClass reflectedClass(s_SEData->AppAssemblyImage, reflectedNamespace, reflectedName);

			// record entity types
			if (reflectedClass.IsSubclassOf(entityClass))
			{
				std::string fullName = reflectedNamespace + "." + reflectedName;
				s_SEData->ScriptClasses[fullName] = Ref<ScriptClass>::Create(reflectedClass);
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// FIND EXPOSED ENTITY TYPE FIELDS
		ScriptClass exposedAttribute(s_SEData->CoreAssemblyImage, "Djinn.CustomAttributes", "ExposedField");
		ScriptClass entityIDAttribute(s_SEData->CoreAssemblyImage, "Djinn.CustomAttributes", "EntityID");

		for (auto& [name, scriptClass] : s_SEData->ScriptClasses)
		{
			void* iterator = nullptr;

			while (MonoClassField* monoField = mono_class_get_fields(scriptClass->m_MonoClass, &iterator))
			{
				// Only fields with the custom [ExposedField] attribute are managed externally
				auto attributes = mono_custom_attrs_from_field(scriptClass->m_MonoClass, monoField);
				if (!attributes)
					continue;
				if (!mono_custom_attrs_has_attr(attributes, exposedAttribute.m_MonoClass))
					continue;

				MonoType* monoType = mono_field_get_type(monoField);
				std::string monoTypeName = mono_type_get_name(monoType);
				auto it = s_SEData->MonoTypeNameToScriptFieldType.find(monoTypeName);
				Z_CORE_ASSERT(it != s_SEData->MonoTypeNameToScriptFieldType.end(), "Unsupported field type");

				auto& field = scriptClass->m_PublicFields.emplace_back();
				field.Type = it->second;
				field.Name = mono_field_get_name(monoField);
				field.MonoField = monoField;

				if (field.Type == ScriptFieldType::uLong && mono_custom_attrs_has_attr(attributes, entityIDAttribute.m_MonoClass))
					field.Type = ScriptFieldType::EntityID;
			}
		}
	}

	Ref<ScriptInstance> ScriptEngine::GetScriptInstance(Entity entity)
	{
		Z_CORE_ASSERT(entity.HasComponents<ScriptComponent>());

		auto& component = entity.GetComponents<ScriptComponent>();
		if (!ValidScriptClass(component.ScriptName))
			return nullptr;

		ZGUID entityGUID = entity.GetGUID();

		auto it = s_SEData->ScriptInstances.find(entityGUID);
		if (it != s_SEData->ScriptInstances.end())
			return it->second;

		return nullptr;
	}

	ScriptClass::ScriptClass(const ScriptClass& other)
	{
		m_MonoClass = other.m_MonoClass;
		m_FullClassName = other.m_FullClassName;
		m_PublicFields = other.m_PublicFields;
	}

	ScriptClass::ScriptClass(MonoImage* image, const std::string& classNamespace, const std::string& className)
		: m_FullClassName(classNamespace + "." + className)
	{
		m_MonoClass = mono_class_from_name(image, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		MonoObject* instance = mono_object_new(s_SEData->AppDomain, m_MonoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& methodName, int numArgs)
	{
		return mono_class_get_method_from_name(m_MonoClass, methodName.c_str(), numArgs);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** args)
	{
		Z_CORE_ASSERT(mono_object_get_class(instance) == m_MonoClass, "Object is not an instance of this class");
		return mono_runtime_invoke(method, instance, args, nullptr); // TODO: expose fourth argument (exception handling)
	}

	bool ScriptClass::IsSubclassOf(ScriptClass& other, bool checkInterfaces)
	{
		return mono_class_is_subclass_of(m_MonoClass, other.GetMonoClass(), checkInterfaces);
	}


	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, ZGUID guid)
		: m_ScriptClass(scriptClass)
	{
		m_MonoObject = m_ScriptClass->Instantiate();

		m_Constructor	= m_ScriptClass->GetMethod(".ctor", 1);
		m_OnCreate		= m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnEarlyUpdate	= m_ScriptClass->GetMethod("OnEarlyUpdate", 1);
		m_OnLateUpdate	= m_ScriptClass->GetMethod("OnLateUpdate", 1);

		Z_CORE_ASSERT(m_Constructor && m_OnCreate && m_OnEarlyUpdate && m_OnLateUpdate,
			"ScriptClass is missing a required method");

		void* args = &guid;
		m_ScriptClass->InvokeMethod(m_MonoObject, m_Constructor, &args);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreate)
			m_ScriptClass->InvokeMethod(m_MonoObject, m_OnCreate, nullptr);
	}

	void ScriptInstance::InvokeEarlyUpdate(float dt)
	{
		void* ptr = &dt;

		if (m_OnEarlyUpdate)
			m_ScriptClass->InvokeMethod(m_MonoObject, m_OnEarlyUpdate, &ptr);
	}

	void ScriptInstance::InvokeLateUpdate(float dt)
	{
		void* ptr = &dt;

		if (m_OnLateUpdate)
			m_ScriptClass->InvokeMethod(m_MonoObject, m_OnLateUpdate, &ptr);
	}

	void ScriptInstance::GetScriptFieldValue(MonoObject* object, MonoClassField* field, void* destination)
	{
		mono_field_get_value(object, field, destination);
	}

	void ScriptInstance::SetScriptFieldValue(MonoObject* object, MonoClassField* field, void* source)
	{
		mono_field_set_value(object, field, source);
	}

}

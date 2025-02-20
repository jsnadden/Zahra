#include "zpch.h"
#include "ScriptEngine.h"

#include "Zahra/Scene/Scene.h"
#include "Zahra/Scripting/ScriptGlue.h"
#include "Zahra/Utils/FileIO.h"

#include "FileWatch.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/tabledefs.h>

namespace Zahra
{
	namespace MonoUtils
	{
		static bool LoadMonoAssembly(const std::filesystem::path& assemblyPath, MonoAssembly*& assembly, MonoImage*& assemblyImage, bool& debugEnabled)
		{
			Z_CORE_ASSERT(std::filesystem::exists(assemblyPath), "Assembly file does not exist");

			uint32_t fileSize = 0;
			char* assemblyFileContents = Zahra::FileIO::ReadBytes(assemblyPath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly, because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* tempImage = mono_image_open_from_data_full(assemblyFileContents, fileSize, 1, &status, 0);

			if (debugEnabled)
			{
				std::filesystem::path debugDatabase = assemblyPath;
				debugDatabase.replace_extension(".pdb");

				if (!std::filesystem::exists(debugDatabase))
				{
					Z_CORE_WARN("Script engine failed to load debug database '{}'. Disabling Mono debugging.",
						debugDatabase.string().c_str());

					debugEnabled = false;
				}

				char* pdbFileContents = Zahra::FileIO::ReadBytes(debugDatabase, &fileSize);

				mono_debug_open_image_from_memory(tempImage, (const mono_byte*)pdbFileContents, fileSize);

				delete[] pdbFileContents;
			}

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				Z_CORE_ERROR("Failed to load MonoImage: {0}", errorMessage);
				return nullptr;
			}

			std::string assemblyPathString = assemblyPath.string();
			assembly = mono_assembly_load_from_full(tempImage, assemblyPathString.c_str(), &status, 0);
			
			mono_image_close(tempImage);
			delete[] assemblyFileContents;

			if (!assembly)
			{
				Z_CORE_ERROR("Script engine failed to load assembly '{}'", assemblyPath.string().c_str());
				return false;
			}

			assemblyImage = mono_assembly_get_image(assembly);
			return true;
		}
	}

	struct ScriptEngineData
	{
		////////////////////
		// Engine core
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
		const std::filesystem::path CoreAssemblyFilepath = "Resources/Scripts/Djinn.dll";
		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		bool EnableFilewatcher = false;
		Scope<filewatch::FileWatch<std::string>> AssemblyFileWatcher;
		bool AssemblyReloadPending = false;
		std::map<uint32_t, std::function<void()>> ReloadCallbacks;

		////////////////////
		// Project-specific
		bool HaveLoadedAppAssembly = false;
		std::filesystem::path AppAssemblyFilepath;
		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;
		std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;

		////////////////////
		// Scene-specific
		Ref<Scene> SceneContext;
		bool SceneRuntime = false;
		std::unordered_map<UUID, Ref<ScriptInstance>> ScriptInstances;

		////////////////////
		// Misc.
#ifdef Z_DEBUG
		bool DebugEnabled = true;
#else
		bool DebugEnabled = false;
#endif

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


	void ScriptEngine::InitCore()
	{
		s_SEData = znew ScriptEngineData();

		CreateRootDomain();
		CreateAppDomain();
		
		bool success = LoadCoreAssembly();
		Z_CORE_ASSERT(success);
		Z_CORE_INFO("Script engine has succesfully loaded core assembly {}", s_SEData->CoreAssemblyFilepath.string().c_str());

		ScriptGlue::RegisterComponentTypes(s_SEData->CoreAssemblyImage);
		ScriptGlue::RegisterFunctions();
	}

	bool ScriptEngine::InitApp(const std::filesystem::path& assemblyFilepath, bool enableAutoReload)
	{
		if (s_SEData->HaveLoadedAppAssembly == true)
			return false;

		s_SEData->AppAssemblyFilepath = assemblyFilepath;
		
		if (LoadAppAssembly())
		{
			if (enableAutoReload)
				InitAssemblyFileWatcher();
			
			RegisterAppEntityScripts();

			Z_CORE_INFO("Script engine has succesfully loaded app assembly '{}'. Auto reloading {}.", assemblyFilepath.string().c_str(), enableAutoReload ? "enabled" : "disabled");
			s_SEData->HaveLoadedAppAssembly = true;

			for (auto& [i, fn] : s_SEData->ReloadCallbacks)
				fn();
		}

		return s_SEData->HaveLoadedAppAssembly;
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMonoDomains();

		delete s_SEData;
	}

	bool ScriptEngine::AppAssemblyAlreadyLoaded()
	{
		return s_SEData->HaveLoadedAppAssembly;
	}

	void ScriptEngine::ReloadAssembly()
	{
		if (!s_SEData->HaveLoadedAppAssembly)
			return;

		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_SEData->AppDomain);

		CreateAppDomain();
		
		bool success = LoadCoreAssembly();
		Z_CORE_ASSERT(success);

		ScriptGlue::RegisterComponentTypes(s_SEData->CoreAssemblyImage);

		if(LoadAppAssembly())
		{
			RegisterAppEntityScripts();

			Z_CORE_INFO("Script engine has successfully reloaded assemblies");
		}

		s_SEData->AssemblyReloadPending = false;

		for (auto& [i, fn] : s_SEData->ReloadCallbacks)
			fn();
	}

	uint64_t ScriptEngine::AddReloadCallback(const std::function<void()>& callback)
	{
		static uint64_t callbackCounter = 0;

		uint64_t receipt = callbackCounter;
		s_SEData->ReloadCallbacks[receipt] = callback;

		callbackCounter++;

		return receipt;
	}

	void ScriptEngine::RemoveReloadCallback(uint64_t receipt)
	{
		Z_CORE_ASSERT(s_SEData->ReloadCallbacks.find(receipt) != s_SEData->ReloadCallbacks.end());

		s_SEData->ReloadCallbacks.erase(receipt);
	}

	void ScriptEngine::CreateRootDomain()
	{
		mono_set_assemblies_path("mono/lib");

		if (s_SEData->DebugEnabled)
		{
			const char* argv[2] =
			{
				"--soft-breakpoints",
				"--debugger-agent=transport=dt_socket,server=y,address=127.0.0.1:55555,server=y,suspend=n,loglevel=3,logfile=Logs/MonoDebug.log"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("ZahraJITRuntime");
		Z_CORE_ASSERT(rootDomain);

		s_SEData->RootDomain = rootDomain;

		if (s_SEData->DebugEnabled)
			mono_debug_domain_create(s_SEData->RootDomain);
	}

	void ScriptEngine::CreateAppDomain()
	{
		s_SEData->AppDomain = mono_domain_create_appdomain("DjinnRuntime", nullptr);
		mono_domain_set(s_SEData->AppDomain, true);
	}

	bool ScriptEngine::LoadCoreAssembly()
	{
		return MonoUtils::LoadMonoAssembly(
			s_SEData->CoreAssemblyFilepath,
			s_SEData->CoreAssembly,
			s_SEData->CoreAssemblyImage,
			s_SEData->DebugEnabled
		);
	}

	bool ScriptEngine::LoadAppAssembly()
	{
		return MonoUtils::LoadMonoAssembly(
			s_SEData->AppAssemblyFilepath,
			s_SEData->AppAssembly,
			s_SEData->AppAssemblyImage,
			s_SEData->DebugEnabled
		);
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

	void ScriptEngine::RegisterAppEntityScripts()
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

	void ScriptEngine::OnRuntimeStart(Ref<Scene> scene)
	{
		s_SEData->SceneContext = scene;
		s_SEData->SceneRuntime = true;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_SEData->SceneRuntime = false;
		s_SEData->SceneContext.Reset();
		s_SEData->ScriptInstances.clear();
		//mono_gc_collect(0); // TODO: maybe trigger garbage collection here
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
			Ref<ScriptInstance> instance = Ref<ScriptInstance>::Create(scriptClass, entity.GetID());
			s_SEData->ScriptInstances[entity.GetID()] = instance;

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

	Entity ScriptEngine::GetEntity(UUID uuid)
	{
		Z_CORE_ASSERT(s_SEData->SceneContext);

		return s_SEData->SceneContext->GetEntity(uuid);
	}

	Entity ScriptEngine::GetEntity(MonoString* name)
	{
		Z_CORE_ASSERT(s_SEData->SceneContext);

		char* string = mono_string_to_utf8(name);
		Entity entity = s_SEData->SceneContext->GetEntity(string);
		mono_free(string);

		return entity;
	}

	MonoObject* ScriptEngine::GetMonoObject(UUID uuid)
	{
		auto it = s_SEData->ScriptInstances.find(uuid);
		if (it != s_SEData->ScriptInstances.end())
			return it->second->GetMonoObject();

		return nullptr;
	}

	MonoString* ScriptEngine::StdStringToMonoString(const std::string& string)
	{
		MonoString* monoString = mono_string_new(s_SEData->AppDomain, string.c_str());
		return monoString;
	}

	Ref<ScriptInstance> ScriptEngine::GetScriptInstance(Entity entity)
	{
		Z_CORE_ASSERT(entity.HasComponents<ScriptComponent>());

		auto& component = entity.GetComponents<ScriptComponent>();
		if (!ValidScriptClass(component.ScriptName))
			return nullptr;

		UUID entityID = entity.GetID();

		auto it = s_SEData->ScriptInstances.find(entityID);
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

		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, args, &exception); // TODO: expose fourth argument (exception handling)
	}

	bool ScriptClass::IsSubclassOf(ScriptClass& other, bool checkInterfaces)
	{
		return mono_class_is_subclass_of(m_MonoClass, other.GetMonoClass(), checkInterfaces);
	}


	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, UUID entityID)
		: m_ScriptClass(scriptClass)
	{
		m_MonoObject = m_ScriptClass->Instantiate();

		m_Constructor	= m_ScriptClass->GetMethod(".ctor", 1);
		m_OnCreate		= m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnEarlyUpdate	= m_ScriptClass->GetMethod("OnEarlyUpdate", 1);
		m_OnLateUpdate	= m_ScriptClass->GetMethod("OnLateUpdate", 1);

		Z_CORE_ASSERT(m_Constructor && m_OnCreate && m_OnEarlyUpdate && m_OnLateUpdate,
			"ScriptClass is missing a required method");

		void* args = &entityID;
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

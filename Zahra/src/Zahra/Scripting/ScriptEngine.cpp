#include "zpch.h"
#include "ScriptEngine.h"

#include "Zahra/Scene/Scene.h"
#include "Zahra/Scripting/ScriptGlue.h"
#include "Zahra/Utils/FileIO.h"

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

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::unordered_map<std::string, Ref<ScriptClass>> ScriptClasses;
		std::vector<const char*> ScriptClassNames;
		std::unordered_map<ZGUID, Ref<ScriptInstance>> ScriptInstances;
		
		//std::map<ZGUID, Buffer> ScriptFieldStorage;

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
			{ "Djinn.Entity",		ScriptFieldType::Entity  },
			{ "Djinn.Vector2",		ScriptFieldType::Vector2 },
			{ "Djinn.Vector3",		ScriptFieldType::Vector3 },
			{ "Djinn.Vector4",		ScriptFieldType::Vector4 },
		};
	};

	static ScriptEngineData* s_SEData;


	void ScriptEngine::Init()
	{
		s_SEData = new ScriptEngineData();

		InitMonoDomains();
		
		// TODO: these paths should be set externally (e.g. via a project system, or in the editor)
		LoadAssembly("../Examples/Bud/Assets/Scripts/Binaries/Djinn.dll", s_SEData->CoreAssembly, s_SEData->CoreAssemblyImage);
		LoadAssembly("../Examples/Bud/Assets/Scripts/Binaries/BudScripts.dll", s_SEData->AppAssembly, s_SEData->AppAssemblyImage);
		
		//ReflectCustomAttributes();
		Reflect();

		ScriptGlue::RegisterComponentTypes(s_SEData->CoreAssemblyImage);
		ScriptGlue::RegisterFunctions();

		Z_CORE_INFO("Script engine initialised");
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMonoDomains();

		delete s_SEData;
	}

	// TODO: once we have intrusive reference counting, we can pass the scene as a Ref instead of a raw pointer
	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_SEData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_SEData->SceneContext = nullptr;
		s_SEData->ScriptInstances.clear();
		//mono_gc_collect(0); // if necessary we can trigger garbage collection here
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

				instance->SetScriptFieldValue<byte>(field, buffer[offset]);
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

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetScriptClasses()
	{
		return s_SEData->ScriptClasses;
	}

	const std::vector<const char*>& ScriptEngine::GetScriptClassNames()
	{
		return s_SEData->ScriptClassNames;
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

	Entity ScriptEngine::GetEntityFromGUID(ZGUID guid)
	{
		Z_CORE_ASSERT(s_SEData->SceneContext);
		return s_SEData->SceneContext->GetEntity(guid);
	}

	MonoString* ScriptEngine::StdStringToMonoString(const std::string& string)
	{
		MonoString* monoString = mono_string_new(s_SEData->AppDomain, string.c_str());
		return monoString;
	}
		
	void ScriptEngine::InitMonoDomains()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("ZahraJITRuntime");
		Z_CORE_ASSERT(rootDomain);

		s_SEData->RootDomain = rootDomain;

		s_SEData->AppDomain = mono_domain_create_appdomain("ZahraScriptRuntime", nullptr);
		mono_domain_set(s_SEData->AppDomain, true);
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& library, MonoAssembly*& assembly, MonoImage*& assemblyImage)
	{
		assembly = MonoUtils::LoadMonoAssembly(library);
		assemblyImage = mono_assembly_get_image(assembly);
	}

	void ScriptEngine::ShutdownMonoDomains()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_SEData->AppDomain);
		s_SEData->AppDomain = nullptr;

		mono_jit_cleanup(s_SEData->RootDomain);
		s_SEData->RootDomain = nullptr;
	}

	/*void ScriptEngine::ReflectCustomAttributes()
	{
		const MonoTableInfo* customAttributeTable = mono_image_get_table_info(s_SEData->AppAssemblyImage, MONO_TABLE_CUSTOMATTRIBUTE);
		int32_t attributeCount = mono_table_info_get_rows(customAttributeTable);

		for (int32_t row = 0; row < attributeCount; row++)
		{
			uint32_t attributeData[MONO_CUSTOM_ATTR_SIZE];
			mono_metadata_decode_row(customAttributeTable, row, attributeData, MONO_CUSTOM_ATTR_SIZE);


		}
	}*/

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

			if (!strcmp(reflectedName.c_str(), "exposed"))
				Z_CORE_INFO("exposed class is in row {}", row);

			// record entity types
			if (reflectedClass.IsSubclassOf(entityClass))
			{
				std::string fullName = reflectedNamespace + "." + reflectedName;
				s_SEData->ScriptClasses[fullName] = Ref<ScriptClass>::Create(reflectedClass);
				s_SEData->ScriptClassNames.push_back(fullName.c_str());
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// FIND EXPOSED ENTITY TYPE FIELDS
		ScriptClass exposedAttribute(s_SEData->CoreAssemblyImage, "Djinn.CustomAttributes", "ExposedField");

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

	/*void ScriptEngine::UpdateScriptFieldStorage(Entity entity)
	{
		Z_CORE_ASSERT(entity.HasComponents<ScriptComponent>());
		auto& component = entity.GetComponents<ScriptComponent>();

		auto& storage = s_SEData->ScriptFieldStorage[s_SEData->SceneContext->GetName()];

		if (ValidScriptClass(component.ScriptName))
		{
			uint64_t fieldCount = s_SEData->ScriptClasses[component.ScriptName]->GetPublicFields().size();

			auto& buffer = storage[entity.GetGUID()];
			if (buffer.GetSize() < 16 * fieldCount)
			{
				buffer.Allocate(16 * fieldCount);
				buffer.ZeroInitialise();
			}
		}
		else
		{
			auto it = storage.find(entity.GetGUID());
			if (it != storage.end())
				it->second.Release();
		}
	}

	void ScriptEngine::FreeScriptFieldStorage(Entity entity)
	{
		auto& storage = s_SEData->ScriptFieldStorage[s_SEData->SceneContext->GetName()];

		auto it = storage.find(entity.GetGUID());
		if (it != storage.end())
			it->second.Release();
	}

	Buffer ScriptEngine::GetScriptFieldStorage(ZGUID guid)
	{
		auto& storage = s_SEData->ScriptFieldStorage[s_SEData->SceneContext->GetName()];

		auto it = storage.find(guid);
		Z_CORE_ASSERT(it != storage.end());

		return it->second;
	}*/

	ScriptClass::ScriptClass(const ScriptClass& other)
	{
		m_MonoClass = other.m_MonoClass;
		m_FullClassName = other.m_FullClassName;
		m_PublicFields = other.m_PublicFields;
		//m_MonoFields = other.m_MonoFields;
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

	/*void ScriptClass::ReflectMethods()
	{
		void* iterator = nullptr;

		Z_CORE_WARN("Methods for class {}:", m_FullClassName);

		while (MonoMethod* monoMethod = mono_class_get_methods(m_MonoClass, &iterator))
		{
			std::string methodName = mono_method_get_name(monoMethod);
			Z_CORE_WARN("	{}", methodName);
		}
	}*/

	bool ScriptClass::IsSubclassOf(ScriptClass& other, bool checkInterfaces)
	{
		return mono_class_is_subclass_of(m_MonoClass, other.GetMonoClass(), checkInterfaces);
	}


	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, ZGUID guid)
		: m_ScriptClass(scriptClass)
	{
		m_MonoObject = m_ScriptClass->Instantiate();

		m_Constructor			= m_ScriptClass->GetMethod(".ctor", 1);
		m_OnCreate				= m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnEarlyUpdate			= m_ScriptClass->GetMethod("OnEarlyUpdate", 1);
		m_OnLateUpdate			= m_ScriptClass->GetMethod("OnLateUpdate", 1);

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

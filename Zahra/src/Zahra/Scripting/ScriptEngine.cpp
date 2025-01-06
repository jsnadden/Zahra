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

namespace Zahra
{
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::unordered_map<std::string, Ref<ScriptEntityType>> ScriptEntityTypes;
		std::unordered_map<ZGUID, Ref<ScriptEntityInstance>> ScriptEntityInstances;

		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_SEData;


	void ScriptEngine::Init()
	{
		s_SEData = new ScriptEngineData();

		InitMonoDomains();
		
		// TODO: these paths should be set externally (e.g. via a project system, or in the editor)
		LoadAssembly("../Examples/Bud/Assets/Scripts/Binaries/Djinn.dll", s_SEData->CoreAssembly, s_SEData->CoreAssemblyImage);
		LoadAssembly("../Examples/Bud/Assets/Scripts/Binaries/BudScripts.dll", s_SEData->AppAssembly, s_SEData->AppAssemblyImage);
		
		ReflectScriptEntities();

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
		s_SEData->ScriptEntityInstances.clear();
		//mono_gc_collect(0); // if necessary we can trigger garbage collection here
	}

	void ScriptEngine::InstantiateScript(Entity entity)
	{
		if (!entity.HasComponents<ScriptComponent>()) return;

		auto& component = entity.GetComponents<ScriptComponent>();
		if (ValidEntityClass(component.ScriptName))
		{
			Ref<ScriptEntityInstance> instance = Ref<ScriptEntityInstance>::Create(s_SEData->ScriptEntityTypes[component.ScriptName], entity.GetGUID());
			s_SEData->ScriptEntityInstances[entity.GetGUID()] = instance;
		}
	}

	void ScriptEngine::UpdateScript(Entity entity, float dt)
	{
		if (!entity.HasComponents<ScriptComponent>()) return;

		auto& component = entity.GetComponents<ScriptComponent>();
		if (!ValidEntityClass(component.ScriptName)) return;

		ZGUID entityGUID = entity.GetGUID();
		Z_CORE_ASSERT(s_SEData->ScriptEntityInstances.find(entityGUID) != s_SEData->ScriptEntityInstances.end(), "Entity not registered with ScriptEngine");
		s_SEData->ScriptEntityInstances[entityGUID]->InvokeOnUpdate(dt);
	}

	std::unordered_map<std::string, Ref<ScriptEntityType>> ScriptEngine::GetEntityTypes()
	{
		return s_SEData->ScriptEntityTypes;
	}

	bool ScriptEngine::ValidEntityClass(const std::string& fullName)
	{
		return s_SEData->ScriptEntityTypes.find(fullName) != s_SEData->ScriptEntityTypes.end();
	}

	Entity ScriptEngine::GetEntity(ZGUID guid)
	{
		Z_CORE_ASSERT(s_SEData->SceneContext);
		return s_SEData->SceneContext->GetEntity(guid);
	}

	MonoString* ScriptEngine::GetMonoString(const std::string& string)
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

	void ScriptEngine::ReflectScriptEntities()
	{
		// re-initialise record of entity types
		s_SEData->ScriptEntityTypes.clear();

		// get typedefs table
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_SEData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		// get entity base class for comparison
		ScriptEntityType entityClass(s_SEData->CoreAssemblyImage, "Djinn", "Entity");

		// iterate over C# types
		for (int32_t i = 0; i < numTypes; i++)
		{
			// get all data in the row
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			std::string reflectedNamespace = mono_metadata_string_heap(s_SEData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			std::string reflectedName = mono_metadata_string_heap(s_SEData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			ScriptEntityType reflectedClass(s_SEData->AppAssemblyImage, reflectedNamespace, reflectedName);

			// record entity types
			if (reflectedClass.IsSubclassOf(entityClass))
			{
				reflectedClass.ReflectFields();

				std::string fullName = reflectedNamespace + "." + reflectedName;
				s_SEData->ScriptEntityTypes[fullName] = Ref<ScriptEntityType>::Create(reflectedClass);
				//Z_CORE_TRACE("{} is an Entity type", fullName);

			}
		}
	}

	ScriptEntityType::ScriptEntityType(const ScriptEntityType& other)
	{
		m_Class = other.m_Class;
		m_Namespace = other.m_Namespace;
		m_Name = other.m_Name;
	}

	ScriptEntityType::ScriptEntityType(MonoImage* image, const std::string& classNamespace, const std::string& className)
		: m_Namespace(classNamespace), m_Name(className)
	{
		m_Class = mono_class_from_name(image, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptEntityType::Instantiate()
	{
		MonoObject* instance = mono_object_new(s_SEData->AppDomain, m_Class);
		mono_runtime_object_init(instance);
		return instance;
	}

	MonoMethod* ScriptEntityType::GetMethod(const std::string& methodName, int numArgs)
	{
		return mono_class_get_method_from_name(m_Class, methodName.c_str(), numArgs);
	}

	MonoObject* ScriptEntityType::InvokeMethod(MonoObject* instance, MonoMethod* method, void** args)
	{
		Z_CORE_ASSERT(mono_object_get_class(instance) == m_Class, "Object is not an instance of this class");
		return mono_runtime_invoke(method, instance, args, nullptr); // TODO: expose fourth argument (exception handling)
	}

	void ScriptEntityType::ReflectFields()
	{
		void* iterator = nullptr;MonoClassField* field;

		while (MonoClassField* monoField = mono_class_get_fields(m_Class, &iterator))
		{
			auto& field = m_Fields.emplace_back();
			field.MonoField = monoField;
			field.Name = mono_field_get_name(monoField);
			auto type = mono_field_get_type(monoField);
		}
	}

	bool ScriptEntityType::IsSubclassOf(ScriptEntityType& other, bool checkInterfaces)
	{
		return mono_class_is_subclass_of(m_Class, other.GetMonoClass(), checkInterfaces);
	}


	ScriptEntityInstance::ScriptEntityInstance(Ref<ScriptEntityType> scriptClass, ZGUID guid)
		: m_Class(scriptClass)
	{
		m_Object = m_Class->Instantiate();

		m_Constructor = m_Class->GetMethod(".ctor", 1);
		m_OnCreate = m_Class->GetMethod("OnCreate", 0);
		m_OnUpdate = m_Class->GetMethod("OnUpdate", 1);

		Z_CORE_ASSERT(m_Constructor && m_OnCreate && m_OnUpdate, "Class is missing a required method");

		void* args = &guid;
		m_Class->InvokeMethod(m_Object, m_Constructor, &args);

		InvokeOnCreate();
	}

	void ScriptEntityInstance::InvokeOnCreate()
	{
		if (m_OnCreate)
			m_Class->InvokeMethod(m_Object, m_OnCreate, nullptr);
	}

	void ScriptEntityInstance::InvokeOnUpdate(float dt)
	{
		void* ptr = &dt;

		if (m_OnUpdate)
			m_Class->InvokeMethod(m_Object, m_OnUpdate, &ptr);
	}

}

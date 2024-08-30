#include "zpch.h"
#include "ScriptEngine.h"

#include "Zahra/Scene/Scene.h"
#include "Zahra/Scripting/ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-gc.h>

namespace MonoUtils
{
	// TODO: move this to a more general engine-wide FileSystem manager:
	static char* ReadBytes(const std::filesystem::path & filepath, uint32_t* outSize)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read((char*)buffer, size);
		stream.close();

		*outSize = size;
		return buffer;
	}

	static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		Z_CORE_ASSERT(std::filesystem::exists(assemblyPath), "Assembly file does not exist");

		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

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

	class ScriptClass
	{
	public:
		ScriptClass() = default;

		ScriptClass(MonoImage* image, const std::string& classNamespace, const std::string& className)
			: m_Namespace(classNamespace), m_Name(className)
		{
			m_Class = mono_class_from_name(image, classNamespace.c_str(), className.c_str());
		}

		MonoObject* Instantiate()
		{
			MonoObject* instance = mono_object_new(mono_domain_get(), m_Class); // TODO: does the domain need to be user-specified?
			mono_runtime_object_init(instance);
			return instance;
		}

		MonoMethod* GetMethod(const std::string& methodName, int numArgs)
		{
			return mono_class_get_method_from_name(m_Class, methodName.c_str(), numArgs);
		}

		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** args)
		{
			Z_CORE_ASSERT(mono_object_get_class(instance) == m_Class, "Object is not an instance of this class");
			return mono_runtime_invoke(method, instance, args, nullptr); // TODO: expose fourth argument (exception handling)
		}

		bool IsSubclassOf(ScriptClass& other, bool checkInterfaces = false)
		{
			return mono_class_is_subclass_of(m_Class, other.GetMonoClass(), checkInterfaces);
		}

		MonoClass* GetMonoClass() { return m_Class; }
		const std::string& GetNamespace() { return m_Namespace; }
		const std::string& GetName() { return m_Name; }
		const std::string& GetFullName() { return m_Namespace + "." + m_Name; }

	private:
		MonoClass* m_Class = nullptr;
		std::string m_Namespace;
		std::string m_Name;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass)
			: m_Class(scriptClass)
		{
			m_Object = m_Class->Instantiate();

			m_OnCreate = m_Class->GetMethod("OnCreate", 0);
			m_OnUpdate = m_Class->GetMethod("OnUpdate", 1);

			InvokeOnCreate();
		}

		void InvokeOnCreate()
		{
			m_Class->InvokeMethod(m_Object, m_OnCreate, nullptr);
		}

		void InvokeOnUpdate(float dt)
		{
			void* ptr = &dt;
			m_Class->InvokeMethod(m_Object, m_OnUpdate, &ptr);
		}

	private:
		Ref<ScriptClass> m_Class;
		MonoObject* m_Object = nullptr;
		MonoMethod* m_OnCreate = nullptr;
		MonoMethod* m_OnUpdate = nullptr;
	};

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityTypes;
		std::unordered_map<ZGUID, Ref<ScriptInstance>> EntityInstances;

		Scene* SceneContext;
	};

	static ScriptEngineData* s_SEData;



	void ScriptEngine::Init()
	{
		s_SEData = new ScriptEngineData;

		InitMonoDomains();
		LoadCoreAssembly("Resources/Scripts/ScriptCore.dll");
		ReflectAssemblyTypes();

		ScriptGlue::RegisterFunctions();

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
		s_SEData->EntityInstances.clear();
		//mono_gc_collect(0); // if necessary we can trigger garbage collection here
	}

	void ScriptEngine::InstantiateScript(Entity entity)
	{
		if (!entity.HasComponents<ScriptComponent>()) return;

		auto& component = entity.GetComponents<ScriptComponent>();
		if (!ValidEntityClass(component.ScriptName)) return;

		s_SEData->EntityInstances[entity.GetGUID()] = CreateRef<ScriptInstance>(s_SEData->EntityTypes[component.ScriptName]);
	}

	void ScriptEngine::UpdateScript(Entity entity, float dt)
	{
		if (!entity.HasComponents<ScriptComponent>()) return;

		auto& component = entity.GetComponents<ScriptComponent>();
		if (!ValidEntityClass(component.ScriptName)) return;

		ZGUID entityGUID = entity.GetGUID();
		Z_CORE_ASSERT(s_SEData->EntityInstances.find(entityGUID) != s_SEData->EntityInstances.end(), "Entity not registered with ScriptEngine");
		s_SEData->EntityInstances[entityGUID]->InvokeOnUpdate(dt);
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityTypes()
	{
		return s_SEData->EntityTypes;
	}

	bool ScriptEngine::ValidEntityClass(const std::string& fullName)
	{
		return s_SEData->EntityTypes.find(fullName) != s_SEData->EntityTypes.end();
	}

	Entity ScriptEngine::GetEntityFromGUID(ZGUID guid)
	{
		return 
	}


		
	void ScriptEngine::InitMonoDomains()
	{
		mono_set_assemblies_path("mono/lib/");

		MonoDomain* rootDomain = mono_jit_init("ZahraJITRuntime");
		Z_CORE_ASSERT(rootDomain);

		s_SEData->RootDomain = rootDomain;

		s_SEData->AppDomain = mono_domain_create_appdomain("ZahraScriptRuntime", nullptr);
		mono_domain_set(s_SEData->AppDomain, true);
	}

	void ScriptEngine::LoadCoreAssembly(std::filesystem::path filepath)
	{
		s_SEData->CoreAssembly = MonoUtils::LoadMonoAssembly(filepath);
		s_SEData->CoreAssemblyImage = mono_assembly_get_image(s_SEData->CoreAssembly);
	}

	void ScriptEngine::ShutdownMonoDomains()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_SEData->AppDomain);
		s_SEData->AppDomain = nullptr;

		mono_jit_cleanup(s_SEData->RootDomain);
		s_SEData->RootDomain = nullptr;
	}

	void ScriptEngine::ReflectAssemblyTypes()
	{
		Z_CORE_TRACE("");
		Z_CORE_TRACE("=================================================================================");
		Z_CORE_TRACE("MONO METADATA REFLECTION");
		Z_CORE_TRACE("---------------------------------------------------------------------------------");

		// re-initialise record of entity types
		s_SEData->EntityTypes.clear();

		// get typedefs table
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_SEData->CoreAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		// get entity base class for comparison
		ScriptClass entityClass(s_SEData->CoreAssemblyImage, "Zahra", "Entity");

		// iterate over rows (i.e. C# types)
		for (int32_t i = 0; i < numTypes; i++)
		{
			// get all data in the row
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			std::string reflectedNamespace = mono_metadata_string_heap(s_SEData->CoreAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			std::string reflectedName = mono_metadata_string_heap(s_SEData->CoreAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			ScriptClass reflectedClass(s_SEData->CoreAssemblyImage, reflectedNamespace, reflectedName);

			// record entity types
			if (reflectedClass.IsSubclassOf(entityClass))
			{
				std::string fullName = reflectedNamespace + "." + reflectedName;
				s_SEData->EntityTypes[fullName] = CreateRef<ScriptClass>(reflectedClass);
				Z_CORE_TRACE("{} is an Entity type", fullName);
			}
		}

		Z_CORE_TRACE("=================================================================================");

	}

		
}

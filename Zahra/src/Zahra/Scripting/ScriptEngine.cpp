#include "zpch.h"
#include "ScriptEngine.h"

#include "Zahra/Scripting/ScriptGlue.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

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

	static void PrintAssemblyTypes(MonoImage* image)
	{
		// TODO: make this a little more presentable

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			Z_CORE_TRACE("{}.{}", nameSpace, name);
		}
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
	};

	static ScriptEngineData* s_Data;

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
			MonoObject* instance = mono_object_new(s_Data->AppDomain, m_Class); // TODO: expose the domain argument?
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

		MonoClass* GetMonoClass() { return m_Class; }
		const std::string& GetNamespace() { return m_Namespace; }
		const std::string& GetName() { return m_Name; }

	private:
		MonoClass* m_Class = nullptr;
		std::string m_Namespace;
		std::string m_Name;
	};



	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData;

		InitMonoDomains();
		LoadCoreAssembly("Resources/Scripts/ScriptCore.dll");

		ScriptGlue::RegisterFunctions();

		////////////////////////////////////////////////////////////////////////////////////
		// TEMP
		ScriptClass exampleClass(s_Data->CoreAssemblyImage, "Zahra", "Example");
		MonoObject* exampleInstance = exampleClass.Instantiate();
		std::string classNamespace = exampleClass.GetNamespace();
		std::string className = exampleClass.GetName();
		Z_CORE_INFO("I got a {}::{}", classNamespace, className);
		MonoMethod* exampleMethod = exampleClass.GetMethod("Hello", 1);
		int n = 7;
		void* ptr = &n;
		exampleClass.InvokeMethod(exampleInstance, exampleMethod, &ptr);

	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMonoDomains();

		delete s_Data;
	}
		
	void ScriptEngine::InitMonoDomains()
	{
		mono_set_assemblies_path("mono/lib/");

		MonoDomain* rootDomain = mono_jit_init("ZahraJITRuntime");
		Z_CORE_ASSERT(rootDomain);

		s_Data->RootDomain = rootDomain;

		s_Data->AppDomain = mono_domain_create_appdomain("ZahraScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);
	}

	void ScriptEngine::LoadCoreAssembly(std::filesystem::path filepath)
	{
		s_Data->CoreAssembly = MonoUtils::LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);

		MonoUtils::PrintAssemblyTypes(s_Data->CoreAssemblyImage);		
	}

	void ScriptEngine::ShutdownMonoDomains()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}


	//// Construct class instance
	//MonoClass* classMain = mono_class_from_name(s_Data->CoreAssemblyImage, "Zahra", "Main");
	//MonoObject* objMain = mono_object_new(s_Data->AppDomain, classMain);
	//mono_runtime_object_init(objMain);

	//// Call C# method
	//MonoMethod* myMethod = mono_class_get_method_from_name(class, methodName, paramCount);
	//mono_runtime_invoke(myMethod, object, args, exceptionHandler);


}

#include "zpch.h"
#include "ScriptEngine.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Zahra
{
	// Since we're interfacing with mono's C API, we can do away
	// with smart pointers (unless something is being exposed)
	
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_Data;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData;

		InitMono();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_Data;
	}

	char* ReadBytes(const std::string& filepath, uint32_t* outSize)
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

	MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath)
	{
		Z_CORE_ASSERT(std::filesystem::exists(assemblyPath), "Assembly file does not exist");

		std::string assemblyPathString = assemblyPath.string();

		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPathString.c_str(), &fileSize);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			Z_CORE_ERROR("Failed to load MonoImage: {0}", errorMessage);
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPathString.c_str(), &status, 0);
		mono_image_close(image);

		// Don't forget to free the file data
		delete[] fileData;

		return assembly;
	}

	void PrintAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
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

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib/");

		MonoDomain* rootDomain = mono_jit_init("ZahraJITRuntime");
		Z_CORE_ASSERT(rootDomain);

		// the only reason we keep this pointer is to delete it later.
		// (you'd think that mono should be able to deal with this internally...)
		s_Data->RootDomain = rootDomain;

		s_Data->AppDomain = mono_domain_create_appdomain("ZahraScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// TEMP:
		s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/ScriptCore.dll");
		PrintAssemblyTypes(s_Data->CoreAssembly);

		MonoImage* imgCore = mono_assembly_get_image(s_Data->CoreAssembly);
		MonoClass* classMain = mono_class_from_name(imgCore, "Zahra", "Main");
		MonoObject* objMain = mono_object_new(s_Data->AppDomain, classMain);
		mono_runtime_object_init(objMain);

		MonoMethod* myMethod = mono_class_get_method_from_name(classMain, "PrintCustomMessage", 1);
		MonoString* message = mono_string_new(s_Data->AppDomain, "send_help_plz...");
		void* args = message;
		mono_runtime_invoke(myMethod, objMain, &args, nullptr);
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	

}

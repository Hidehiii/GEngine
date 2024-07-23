#include "GEpch.h"
#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace GEngine
{
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		InitMono();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	char* ReadBytes(const std::string& filePath, uint32_t* outSize)
	{
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);

		if (!file)
		{
			return nullptr;
		}

		std::streampos end = file.tellg();
		file.seekg(0, std::ios::beg);
		uint32_t size = (uint32_t)(end - file.tellg());

		if (size == 0)
		{
			return nullptr;
		}

		char* buffer = new char[size];
		file.read(buffer, size);
		file.close();

		*outSize = size;
		return buffer;
	}

	MonoAssembly* LoadCSharpAssembly(const std::string& filePath)
	{
		uint32_t size = 0;
		char* data = ReadBytes(filePath, &size);
		if (!data)
		{
			GE_CORE_WARN("Failed to read assembly file '{0}'", filePath);
			return nullptr;
		}

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(data, size, 1, &status, 0);
		if (status != MONO_IMAGE_OK)
		{
			const char* error = mono_image_strerror(status);
			GE_CORE_ERROR(error);
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, filePath.c_str(), &status, 0);
		mono_image_close(image);

		delete[] data;
		return assembly;
	}

	void PrintAssemblyType(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* tableInfo = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t rows = mono_table_info_get_rows(tableInfo);

		for (int32_t i = 0; i < rows; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(tableInfo, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			GE_CORE_INFO("{0}.{1}", nameSpace, name);
		}
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");
		MonoDomain* rootDomain = mono_jit_init("GEngineJITRuntime");
		GE_CORE_ASSERT(rootDomain, "Failed to initialize Mono runtime");
		s_Data->RootDomain = rootDomain;
		s_Data->AppDomain = mono_domain_create_appdomain("GEngineScriptRuntime", nullptr);
		GE_CORE_ASSERT(s_Data->AppDomain, "Failed to create app domain");
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/GEngine-ScriptCore.dll");
		PrintAssemblyType(s_Data->CoreAssembly);

		// Create an obj
		MonoImage* assemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		MonoClass* objClass = mono_class_from_name(assemblyImage, "GEngine", "Main");

		MonoObject* instance = mono_object_new(s_Data->AppDomain, objClass);
		mono_runtime_object_init(instance);
		// Call a method
		MonoMethod* methodPrintInts = mono_class_get_method_from_name(objClass, "PrintInts", 2);
		int v1 = 1, v2 = 500;
		void* params[2] =
		{
			&v1,
			&v2
		};
		mono_runtime_invoke(methodPrintInts, instance, params, nullptr);

		MonoMethod* methodPrintString = mono_class_get_method_from_name(objClass, "PrintString", 1);
		MonoString* monoString = mono_string_new(s_Data->AppDomain, "Hello from C++!");
		void* stringParams[1] = { monoString };
		mono_runtime_invoke(methodPrintString, instance, stringParams, nullptr);

	}
	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(s_Data->RootDomain, true);
		mono_domain_unload(s_Data->AppDomain);
		mono_jit_cleanup(s_Data->RootDomain);
	}

}
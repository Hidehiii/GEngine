#include "GEpch.h"
#include "GEngine/Scripting/ScriptEngine.h"
#include "GEngine/Runtime/EngineRuntime.h"

#ifndef GE_WITH_MONO
#define GE_WITH_MONO 1
#endif
#if GE_WITH_MONO

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <filesystem>
#include <limits>
#include <memory>
#include <system_error>

namespace GEngine
{
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	bool ScriptEngine::Init(const RuntimeSpecification& specification)
	{
		if (s_Data)
		{
			GE_CORE_ERROR("Script runtime is already initialized");
			return false;
		}

		s_Data = new ScriptEngineData();
		if (!InitMono(specification))
		{
			ShutdownMono();
			delete s_Data;
			s_Data = nullptr;
			return false;
		}
		return true;
	}

	void ScriptEngine::Shutdown()
	{
		if (!s_Data) return;

		ShutdownMono();
		delete s_Data;
		s_Data = nullptr;
	}

	char* ReadBytes(const std::filesystem::path& filePath, uint32_t* outSize)
	{
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file)
		{
			return nullptr;
		}

		const std::streampos end = file.tellg();
		if (end < 0)
		{
			GE_CORE_WARN("Failed to determine assembly size '{0}'", filePath.string());
			return nullptr;
		}

		file.seekg(0, std::ios::beg);
		const auto length = static_cast<std::uintmax_t>(end);
		if (length == 0 || length > std::numeric_limits<uint32_t>::max())
		{
			GE_CORE_WARN("Invalid assembly size '{0}'", filePath.string());
			return nullptr;
		}

		const auto size = static_cast<uint32_t>(length);
		std::unique_ptr<char[]> buffer(new char[size]);
		file.read(buffer.get(), size);
		file.close();
		if (!file)
		{
			GE_CORE_WARN("Failed to read assembly file '{0}'", filePath.string());
			return nullptr;
		}

		*outSize = size;
		return buffer.release();
	}

	MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& filePath)
	{
		uint32_t size = 0;
		std::unique_ptr<char[]> data(ReadBytes(filePath, &size));
		if (!data)
		{
			GE_CORE_WARN("Failed to read assembly file '{0}'", filePath.string());
			return nullptr;
		}

		MonoImageOpenStatus status = MONO_IMAGE_OK;
		MonoImage* image = mono_image_open_from_data_full(data.get(), size, 1, &status, 0);
		if (!image || status != MONO_IMAGE_OK)
		{
			GE_CORE_ERROR("Failed to open assembly image '{0}': {1}",
				filePath.string(), mono_image_strerror(status));
			return nullptr;
		}

		const std::string assemblyPath = filePath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);
		if (!assembly || status != MONO_IMAGE_OK)
		{
			GE_CORE_ERROR("Failed to load assembly '{0}': {1}",
				assemblyPath, mono_image_strerror(status));
			return nullptr;
		}
		return assembly;
	}

	void PrintAssemblyType(MonoAssembly* assembly)
	{
		if (!assembly) return;

		MonoImage* image = mono_assembly_get_image(assembly);
		if (!image) return;

		const MonoTableInfo* tableInfo = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		if (!tableInfo) return;

		const int32_t rows = mono_table_info_get_rows(tableInfo);
		for (int32_t i = 0; i < rows; ++i)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(tableInfo, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			GE_CORE_INFO("{0}.{1}", nameSpace, name);
		}
	}

	bool ScriptEngine::InitMono(const RuntimeSpecification& specification)
	{
		std::error_code error;
		if (specification.ScriptRuntimeRoot.empty())
		{
			GE_CORE_ERROR("Script runtime root is empty");
			return false;
		}
		if (!std::filesystem::is_directory(specification.ScriptRuntimeRoot, error))
		{
			GE_CORE_ERROR("Script runtime root is not a directory: {0}",
				specification.ScriptRuntimeRoot.string());
			return false;
		}
		if (specification.ScriptCoreAssembly.empty())
		{
			GE_CORE_ERROR("Script core assembly path is empty");
			return false;
		}
		if (!std::filesystem::is_regular_file(specification.ScriptCoreAssembly, error))
		{
			GE_CORE_ERROR("Script core assembly is not a file: {0}",
				specification.ScriptCoreAssembly.string());
			return false;
		}

		const std::string runtimeRoot = specification.ScriptRuntimeRoot.string();
		mono_set_assemblies_path(runtimeRoot.c_str());

		MonoDomain* rootDomain = mono_jit_init("GEngineJITRuntime");
		if (!rootDomain)
		{
			GE_CORE_ERROR("Failed to initialize Mono runtime");
			return false;
		}
		s_Data->RootDomain = rootDomain;

		MonoDomain* appDomain = mono_domain_create_appdomain("GEngineScriptRuntime", nullptr);
		if (!appDomain)
		{
			GE_CORE_ERROR("Failed to create Mono application domain");
			return false;
		}
		s_Data->AppDomain = appDomain;
		mono_domain_set(appDomain, true);

		MonoAssembly* coreAssembly = LoadCSharpAssembly(specification.ScriptCoreAssembly);
		if (!coreAssembly)
		{
			GE_CORE_ERROR("Failed to load script core assembly: {0}",
				specification.ScriptCoreAssembly.string());
			return false;
		}
		s_Data->CoreAssembly = coreAssembly;
		PrintAssemblyType(coreAssembly);

		MonoImage* assemblyImage = mono_assembly_get_image(coreAssembly);
		if (!assemblyImage)
		{
			GE_CORE_ERROR("Failed to get script core assembly image");
			return false;
		}

		MonoClass* objClass = mono_class_from_name(assemblyImage, "GEngine", "Main");
		if (!objClass)
		{
			GE_CORE_ERROR("Failed to find GEngine.Main in script core assembly");
			return false;
		}

		MonoObject* instance = mono_object_new(appDomain, objClass);
		if (!instance)
		{
			GE_CORE_ERROR("Failed to create GEngine.Main instance");
			return false;
		}
		mono_runtime_object_init(instance);

		MonoMethod* methodPrintInts = mono_class_get_method_from_name(objClass, "PrintInts", 2);
		if (!methodPrintInts)
		{
			GE_CORE_ERROR("Failed to find GEngine.Main::PrintInts");
			return false;
		}

		int v1 = 1, v2 = 500;
		void* params[2] = { &v1, &v2 };
		MonoObject* exception = nullptr;
		mono_runtime_invoke(methodPrintInts, instance, params, &exception);
		if (exception)
		{
			GE_CORE_ERROR("GEngine.Main::PrintInts threw a managed exception");
			return false;
		}

		MonoMethod* methodPrintString = mono_class_get_method_from_name(objClass, "PrintString", 1);
		if (!methodPrintString)
		{
			GE_CORE_ERROR("Failed to find GEngine.Main::PrintString");
			return false;
		}

		MonoString* monoString = mono_string_new(appDomain, "Hello from C++!");
		if (!monoString)
		{
			GE_CORE_ERROR("Failed to create Mono string");
			return false;
		}

		void* stringParams[1] = { monoString };
		mono_runtime_invoke(methodPrintString, instance, stringParams, &exception);
		if (exception)
		{
			GE_CORE_ERROR("GEngine.Main::PrintString threw a managed exception");
			return false;
		}

		return true;
	}

	void ScriptEngine::ShutdownMono()
	{
		if (!s_Data) return;

		if (s_Data->AppDomain)
		{
			if (s_Data->RootDomain) mono_domain_set(s_Data->RootDomain, true);
			mono_domain_unload(s_Data->AppDomain);
		}
		if (s_Data->RootDomain) mono_jit_cleanup(s_Data->RootDomain);

		s_Data->RootDomain = nullptr;
		s_Data->AppDomain = nullptr;
		s_Data->CoreAssembly = nullptr;
	}
}
#else
namespace GEngine
{
    bool ScriptEngine::Init(const RuntimeSpecification&)
    { GE_CORE_ERROR("This engine was built without managed scripting support."); return false; }
    void ScriptEngine::Shutdown() {}
}
#endif

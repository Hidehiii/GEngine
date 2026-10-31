#include "GEpch.h"
#include "GEngine/Scripting/ScriptEngine.h"
#include "GEngine/Scripting/NativeScriptBehaviour.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Runtime/EngineRuntime.h"

namespace GEngine
{
	using NativeScriptFactory = std::function<std::unique_ptr<NativeScriptBehaviour>()>;
}

#ifndef GE_WITH_MONO
#define GE_WITH_MONO 1
#endif
#if GE_WITH_MONO

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

#include <algorithm>
#include <filesystem>
#include <limits>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <unordered_map>

namespace GEngine
{
	struct ScriptObjectKey
	{
		const void* Scene = nullptr;
		std::uint64_t UUID = 0;

		bool operator==(const ScriptObjectKey& other) const
		{ return Scene == other.Scene && UUID == other.UUID; }
	};

	struct ScriptObjectKeyHash
	{
		std::size_t operator()(const ScriptObjectKey& key) const
		{
			return std::hash<const void*>()(key.Scene) ^ (std::hash<std::uint64_t>()(key.UUID) << 1);
		}
	};

	struct ScriptInstanceKey
	{
		const void* Scene = nullptr;
		std::uint64_t UUID = 0;
		std::string ClassName;

		bool operator==(const ScriptInstanceKey& other) const
		{
			return Scene == other.Scene && UUID == other.UUID && ClassName == other.ClassName;
		}
	};

	struct ScriptInstanceKeyHash
	{
		std::size_t operator()(const ScriptInstanceKey& key) const
		{
			return std::hash<const void*>()(key.Scene) ^ (std::hash<std::uint64_t>()(key.UUID) << 1) ^
				(std::hash<std::string>()(key.ClassName) << 2);
		}
	};

	struct ScriptInstance
	{
		MonoClass* Class = nullptr;
		std::uint32_t Handle = 0;
		bool Awake = false;
		bool Started = false;
		bool Failed = false;
	};

	struct NativeScriptInstance
	{
		std::unique_ptr<NativeScriptBehaviour> Instance;
		bool Failed = false;
	};

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreImage = nullptr;
		MonoClass* ScriptBehaviourClass = nullptr;
		MonoClass* GameObjectClass = nullptr;
		MonoMethod* GameObjectConstructor = nullptr;
		MonoMethod* GameObjectUpdateMethod = nullptr;
		MonoClassField* GameObjectField = nullptr;
		std::unordered_map<ScriptObjectKey, std::uint32_t, ScriptObjectKeyHash> GameObjectHandles;
		std::unordered_map<ScriptInstanceKey, ScriptInstance, ScriptInstanceKeyHash> Scripts;
	};

	static ScriptEngineData* s_Data = nullptr;
	static std::unordered_map<std::string, NativeScriptFactory> s_NativeFactories;
	static std::unordered_map<ScriptInstanceKey, NativeScriptInstance, ScriptInstanceKeyHash> s_NativeScripts;

	static ScriptObjectKey ObjectKey(const GameObject& object)
	{
		Scene* scene = object.GetScene();
		return { scene, static_cast<std::uint64_t>(object.GetUUID()) };
	}

	static ScriptInstanceKey InstanceKey(const GameObject& object, const std::string& className)
	{
		Scene* scene = object.GetScene();
		return { scene, static_cast<std::uint64_t>(object.GetUUID()), className };
	}

	static bool SameObject(const ScriptInstanceKey& key, GameObject& object)
	{
		Scene* scene = object.GetScene();
		return key.Scene == scene && key.UUID == static_cast<std::uint64_t>(object.GetUUID());
	}

	static std::string ManagedExceptionMessage(MonoObject* exception)
	{
		if (!exception) return {};
		MonoObject* toStringException = nullptr;
		MonoString* text = mono_object_to_string(exception, &toStringException);
		if (!text || toStringException) return "managed exception could not be formatted";
		char* utf8 = mono_string_to_utf8(text);
		std::string result = utf8 ? utf8 : "managed exception could not be formatted";
		if (utf8) mono_free(utf8);
		return result;
	}

	static bool InvokeScriptMethod(ScriptInstance& instance, const char* methodName,
		int parameterCount, void** parameters, bool allowFailed = false)
	{
		if ((!allowFailed && instance.Failed) || !instance.Class || !instance.Handle) return false;
		MonoMethod* method = mono_class_get_method_from_name(instance.Class, methodName, parameterCount);
		if (!method) return true;
		MonoObject* instanceObject = mono_gchandle_get_target(instance.Handle);
		if (!instanceObject) return false;
		MonoObject* exception = nullptr;
		mono_runtime_invoke(method, instanceObject, parameters, &exception);
		if (exception)
		{
			instance.Failed = true;
			GE_CORE_ERROR("Script method {0} failed: {1}", methodName, ManagedExceptionMessage(exception));
			return false;
		}
		return true;
	}

	static bool InvokeNativeScriptMethod(NativeScriptInstance& instance, const char* methodName,
		const std::function<void(NativeScriptBehaviour&)>& method, bool allowFailed = false)
	{
		if ((!allowFailed && instance.Failed) || !instance.Instance) return false;
		try
		{
			method(*instance.Instance);
			return true;
		}
		catch (const std::exception& error)
		{
			instance.Failed = true;
			GE_CORE_ERROR("Native script method {0} failed: {1}", methodName, error.what());
			return false;
		}
		catch (...)
		{
			instance.Failed = true;
			GE_CORE_ERROR("Native script method {0} failed with an unknown exception", methodName);
			return false;
		}
	}

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
			if (!s_NativeFactories.empty())
			{
				GE_CORE_WARN("Managed script runtime unavailable; native C++ scripts remain available");
				return true;
			}
			return false;
		}
		return true;
	}

	void ScriptEngine::Shutdown()
	{
		for (auto& [key, instance] : s_NativeScripts)
			InvokeNativeScriptMethod(instance, "OnDestroy", [](NativeScriptBehaviour& script) { script.OnDestroy(); }, true);
		s_NativeScripts.clear();

		if (s_Data)
		{
			for (auto& [key, instance] : s_Data->Scripts)
			{
				InvokeScriptMethod(instance, "OnDestroy", 0, nullptr, true);
				if (instance.Handle) mono_gchandle_free(instance.Handle);
			}
			for (auto& [key, handle] : s_Data->GameObjectHandles)
				if (handle) mono_gchandle_free(handle);
			ShutdownMono();
			delete s_Data;
			s_Data = nullptr;
		}
	}

	bool ScriptEngine::IsInitialized() { return s_Data != nullptr; }

	bool ScriptEngine::RegisterNativeScript(const std::string& className, NativeScriptFactory factory)
	{
		if (className.empty() || !factory)
		{
			GE_CORE_ERROR("Native script registration requires a class name and factory");
			return false;
		}
		if (s_NativeFactories.count(className))
		{
			GE_CORE_ERROR("Native script class is already registered: {0}", className);
			return false;
		}
		s_NativeFactories.emplace(className, std::move(factory));
		return true;
	}

	bool ScriptEngine::HasNativeScript(const std::string& className)
	{ return s_NativeFactories.count(className) != 0; }

	static MonoClass* FindScriptBehaviourClass(const std::string& className)
	{
		const auto separator = className.rfind('.');
		const auto namespaceName = separator == std::string::npos ? std::string{} : className.substr(0, separator);
		const auto typeName = separator == std::string::npos ? className : className.substr(separator + 1);
		MonoClass* result = mono_class_from_name(s_Data->CoreImage, namespaceName.c_str(), typeName.c_str());
		if (!result)
			throw std::runtime_error("Script class is missing from the core assembly: " + className);
		if (!mono_class_is_subclass_of(result, s_Data->ScriptBehaviourClass, false))
			throw std::runtime_error("Script class does not derive from GEngine.ScriptBehaviour: " + className);
		return result;
	}

	static std::uint32_t EnsureGameObjectHandle(GameObject& object)
	{
		const auto key = ObjectKey(object);
		const auto found = s_Data->GameObjectHandles.find(key);
		if (found != s_Data->GameObjectHandles.end()) return found->second;

		MonoObject* wrapper = mono_object_new(s_Data->AppDomain, s_Data->GameObjectClass);
		if (!wrapper) throw std::runtime_error("Failed to create a managed GameObject wrapper.");
		MonoMethod* constructor = mono_class_get_method_from_name(s_Data->GameObjectClass, ".ctor", 3);
		if (!constructor) throw std::runtime_error("Managed GameObject constructor is missing.");
		auto uuid = static_cast<std::uint64_t>(object.GetUUID());
		MonoString* name = mono_string_new(s_Data->AppDomain, object.GetName().c_str());
		int active = object.IsActive() ? 1 : 0;
		void* parameters[3] = { &uuid, name, &active };
		MonoObject* exception = nullptr;
		mono_runtime_invoke(constructor, wrapper, parameters, &exception);
		if (exception)
			throw std::runtime_error("Managed GameObject construction failed: " + ManagedExceptionMessage(exception));
		const auto handle = mono_gchandle_new(wrapper, false);
		s_Data->GameObjectHandles.emplace(key, handle);
		return handle;
	}

	static void UpdateGameObjectHandle(GameObject& object)
	{
		const auto handle = EnsureGameObjectHandle(object);
		MonoObject* wrapper = mono_gchandle_get_target(handle);
		if (!wrapper) throw std::runtime_error("Managed GameObject wrapper was collected unexpectedly.");
		MonoString* name = mono_string_new(s_Data->AppDomain, object.GetName().c_str());
		int active = object.IsActive() ? 1 : 0;
		void* parameters[2] = { name, &active };
		MonoObject* exception = nullptr;
		mono_runtime_invoke(s_Data->GameObjectUpdateMethod, wrapper, parameters, &exception);
		if (exception)
			throw std::runtime_error("Managed GameObject update failed: " + ManagedExceptionMessage(exception));
	}

	static void SetScriptGameObject(ScriptInstance& instance, GameObject& object)
	{
		const auto objectHandle = EnsureGameObjectHandle(object);
		MonoObject* wrapper = mono_gchandle_get_target(objectHandle);
		MonoObject* scriptObject = mono_gchandle_get_target(instance.Handle);
		if (!wrapper || !scriptObject)
			throw std::runtime_error("Managed script or GameObject wrapper is unavailable.");
		mono_field_set_value(scriptObject, s_Data->GameObjectField, wrapper);
	}

	static void CreateNativeScript(GameObject& object, const std::string& className)
	{
		const auto factory = s_NativeFactories.find(className);
		if (factory == s_NativeFactories.end())
			throw std::runtime_error("Native script factory is missing: " + className);
		auto instance = factory->second();
		if (!instance)
			throw std::runtime_error("Native script factory returned null: " + className);
		instance->SetGameObject(object);
		const auto key = InstanceKey(object, className);
		auto [inserted, unused] = s_NativeScripts.emplace(key, NativeScriptInstance{std::move(instance), false});
		(void)unused;
		InvokeNativeScriptMethod(inserted->second, "OnAwake",
			[](NativeScriptBehaviour& script) { script.OnAwake(); });
	}

	void ScriptEngine::OnGameObjectAwake(GameObject& object)
	{
		for (const auto& className : object.GetScriptClasses())
		{
			if (HasNativeScript(className))
			{
				CreateNativeScript(object, className);
				continue;
			}
			if (!s_Data)
				throw std::runtime_error("Neither a native script nor the managed runtime is available for: " + className);
			const auto key = InstanceKey(object, className);
			if (s_Data->Scripts.count(key)) continue;
			UpdateGameObjectHandle(object);
			MonoClass* scriptClass = FindScriptBehaviourClass(className);
			MonoObject* instanceObject = mono_object_new(s_Data->AppDomain, scriptClass);
			if (!instanceObject) throw std::runtime_error("Failed to create script instance: " + className);
			MonoMethod* constructor = mono_class_get_method_from_name(scriptClass, ".ctor", 0);
			if (!constructor) throw std::runtime_error("Script default constructor is missing: " + className);
			MonoObject* exception = nullptr;
			mono_runtime_invoke(constructor, instanceObject, nullptr, &exception);
			if (exception)
				throw std::runtime_error("Script constructor failed: " + ManagedExceptionMessage(exception));
			ScriptInstance instance{};
			instance.Class = scriptClass;
			instance.Handle = mono_gchandle_new(instanceObject, false);
			SetScriptGameObject(instance, object);
			auto [inserted, unused] = s_Data->Scripts.emplace(key, std::move(instance));
			(void)unused;
			InvokeScriptMethod(inserted->second, "OnAwake", 0, nullptr);
			if (!inserted->second.Failed) inserted->second.Awake = true;
		}
	}

	void ScriptEngine::OnGameObjectStart(GameObject& object)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnStart",
				[](NativeScriptBehaviour& script) { script.OnStart(); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			if (InvokeScriptMethod(instance, "OnStart", 0, nullptr) && !instance.Started)
				instance.Started = true;
		}
	}

	void ScriptEngine::OnGameObjectEnable(GameObject& object)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnEnable",
				[](NativeScriptBehaviour& script) { script.OnEnable(); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			InvokeScriptMethod(instance, "OnEnable", 0, nullptr);
		}
	}

	void ScriptEngine::OnGameObjectDisable(GameObject& object)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnDisable",
				[](NativeScriptBehaviour& script) { script.OnDisable(); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			InvokeScriptMethod(instance, "OnDisable", 0, nullptr);
		}
	}

	void ScriptEngine::OnGameObjectFixedUpdate(GameObject& object, float fixedDeltaTime)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnFixedUpdate",
				[fixedDeltaTime](NativeScriptBehaviour& script) { script.OnFixedUpdate(fixedDeltaTime); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			void* parameters[1] = { &fixedDeltaTime };
			InvokeScriptMethod(instance, "OnFixedUpdate", 1, parameters);
		}
	}

	void ScriptEngine::OnGameObjectUpdate(GameObject& object, float deltaTime)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnUpdate",
				[deltaTime](NativeScriptBehaviour& script) { script.OnUpdate(deltaTime); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			void* parameters[1] = { &deltaTime };
			InvokeScriptMethod(instance, "OnUpdate", 1, parameters);
		}
	}

	void ScriptEngine::OnGameObjectLateUpdate(GameObject& object)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnLateUpdate",
				[](NativeScriptBehaviour& script) { script.OnLateUpdate(); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			InvokeScriptMethod(instance, "OnLateUpdate", 0, nullptr);
		}
	}

	void ScriptEngine::OnGameObjectPause(GameObject& object)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnPause",
				[](NativeScriptBehaviour& script) { script.OnPause(); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			InvokeScriptMethod(instance, "OnPause", 0, nullptr);
		}
	}

	void ScriptEngine::OnGameObjectResume(GameObject& object)
	{
		for (auto& [key, instance] : s_NativeScripts)
		{
			if (!SameObject(key, object)) continue;
			InvokeNativeScriptMethod(instance, "OnResume",
				[](NativeScriptBehaviour& script) { script.OnResume(); });
		}
		if (!s_Data) return;
		UpdateGameObjectHandle(object);
		for (auto& [key, instance] : s_Data->Scripts)
		{
			if (!SameObject(key, object)) continue;
			SetScriptGameObject(instance, object);
			InvokeScriptMethod(instance, "OnResume", 0, nullptr);
		}
	}

	void ScriptEngine::OnGameObjectDestroy(GameObject& object)
	{
		const auto objectKey = ObjectKey(object);
		for (auto it = s_NativeScripts.begin(); it != s_NativeScripts.end();)
		{
			if (it->first.Scene != objectKey.Scene || it->first.UUID != objectKey.UUID)
			{
				++it;
				continue;
			}
			InvokeNativeScriptMethod(it->second, "OnDestroy",
				[](NativeScriptBehaviour& script) { script.OnDestroy(); }, true);
			it = s_NativeScripts.erase(it);
		}
		if (!s_Data) return;
		for (auto it = s_Data->Scripts.begin(); it != s_Data->Scripts.end();)
		{
			if (it->first.Scene != objectKey.Scene || it->first.UUID != objectKey.UUID)
			{
				++it;
				continue;
			}
			UpdateGameObjectHandle(object);
			SetScriptGameObject(it->second, object);
			InvokeScriptMethod(it->second, "OnDestroy", 0, nullptr, true);
			if (it->second.Handle) mono_gchandle_free(it->second.Handle);
			it = s_Data->Scripts.erase(it);
		}
		const auto wrapper = s_Data->GameObjectHandles.find(objectKey);
		if (wrapper != s_Data->GameObjectHandles.end())
		{
			if (wrapper->second) mono_gchandle_free(wrapper->second);
			s_Data->GameObjectHandles.erase(wrapper);
		}
	}

	void ScriptEngine::RemoveScript(GameObject& object, const std::string& className)
	{
		const auto key = InstanceKey(object, className);
		const auto native = s_NativeScripts.find(key);
		if (native != s_NativeScripts.end())
		{
			InvokeNativeScriptMethod(native->second, "OnDestroy",
				[](NativeScriptBehaviour& script) { script.OnDestroy(); }, true);
			s_NativeScripts.erase(native);
			return;
		}
		if (!s_Data) return;
		const auto found = s_Data->Scripts.find(key);
		if (found == s_Data->Scripts.end()) return;
		UpdateGameObjectHandle(object);
		SetScriptGameObject(found->second, object);
		InvokeScriptMethod(found->second, "OnDestroy", 0, nullptr, true);
		if (found->second.Handle) mono_gchandle_free(found->second.Handle);
		s_Data->Scripts.erase(found);
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
		s_Data->CoreImage = assemblyImage;

		MonoClass* mainClass = mono_class_from_name(assemblyImage, "GEngine", "Main");
		if (!mainClass)
		{
			GE_CORE_ERROR("Failed to find GEngine.Main in script core assembly");
			return false;
		}

		s_Data->ScriptBehaviourClass = mono_class_from_name(assemblyImage, "GEngine", "ScriptBehaviour");
		s_Data->GameObjectClass = mono_class_from_name(assemblyImage, "GEngine", "GameObject");
		if (!s_Data->ScriptBehaviourClass || !s_Data->GameObjectClass)
		{
			GE_CORE_ERROR("Script core assembly is missing GEngine.ScriptBehaviour or GEngine.GameObject");
			return false;
		}
		s_Data->GameObjectField = mono_class_get_field_from_name(s_Data->ScriptBehaviourClass, "GameObject");
		s_Data->GameObjectConstructor = mono_class_get_method_from_name(s_Data->GameObjectClass, ".ctor", 3);
		s_Data->GameObjectUpdateMethod = mono_class_get_method_from_name(s_Data->GameObjectClass, "Update", 2);
		if (!s_Data->GameObjectField || !s_Data->GameObjectConstructor || !s_Data->GameObjectUpdateMethod)
		{
			GE_CORE_ERROR("Script core GameObject wrapper contract is incomplete");
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
		s_Data->CoreImage = nullptr;
		s_Data->ScriptBehaviourClass = nullptr;
		s_Data->GameObjectClass = nullptr;
		s_Data->GameObjectConstructor = nullptr;
		s_Data->GameObjectUpdateMethod = nullptr;
		s_Data->GameObjectField = nullptr;
		s_Data->GameObjectHandles.clear();
		s_Data->Scripts.clear();
	}
}
#else
namespace GEngine
{
    static void RequireScriptRuntimeStub(GameObject& object)
    {
        if (object.GetScriptClasses().empty()) return;
        throw std::runtime_error("This engine was built without managed scripting support.");
    }

    bool ScriptEngine::Init(const RuntimeSpecification&)
    { GE_CORE_ERROR("This engine was built without managed scripting support."); return false; }
    void ScriptEngine::Shutdown() {}
    bool ScriptEngine::IsInitialized() { return false; }
    bool ScriptEngine::RegisterNativeScript(const std::string&, NativeScriptFactory)
    {
        GE_CORE_ERROR("Native C++ scene scripts require the scripting-enabled engine build.");
        return false;
    }
    bool ScriptEngine::HasNativeScript(const std::string&) { return false; }
    void ScriptEngine::OnGameObjectAwake(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectStart(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectEnable(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectDisable(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectFixedUpdate(GameObject& object, float) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectUpdate(GameObject& object, float) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectLateUpdate(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectPause(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectResume(GameObject& object) { RequireScriptRuntimeStub(object); }
    void ScriptEngine::OnGameObjectDestroy(GameObject&) {}
    void ScriptEngine::RemoveScript(GameObject& object, const std::string&) { RequireScriptRuntimeStub(object); }
}
#endif

#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/ScriptableObject.h"
#include "GEngine/Components/Component.h"
namespace GEngine
{
	// Base class for all native scripts
	// Each native script is attached to a game object
	// The native script is used to create custom scripts
	class GENGINE_API NativeScript : public Component
	{
	public:
		NativeScript(const NativeScript&) = default;

		ScriptableObject*(*InstantiateFunc)();
		void (*DestroyFunc)(NativeScript*);

		template<typename T>
		void Bind()
		{
			InstantiateFunc = [](){ return static_cast<ScriptableObject*>(new T()); };
			DestroyFunc = [](NativeScript* ns) { delete ns->Instance; ns->Instance = nullptr; };
		}

		template<typename T>
		void AddInstance()
		{
			m_Functions.emplace_back([]() { return static_cast<ScriptableObject*>(new T()); }, 
									[](NativeScript* ns) { delete ns->Instance; ns->Instance = nullptr; });
			m_Scripts.emplace_back(nullptr);
		}

		template<typename T>
		void GetInstance()
		{

		}

		template<typename T>
		void RemoveInstance()
		{

		}
	public:
		ScriptableObject* Instance = nullptr;
		std::vector < std::pair<ScriptableObject* (*)(), void (*)(NativeScript*)>>		m_Functions;
		std::vector<ScriptableObject*>													m_Scripts;
	};
}



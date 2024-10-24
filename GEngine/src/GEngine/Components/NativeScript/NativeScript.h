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
		NativeScript() = default;
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
		T* AddInstance()
		{
			GE_CORE_ASSERT(GetInstance<T>() == nullptr, "There is a same script has been added!");
			T* script = new T();
			m_Scripts.emplace_back(static_cast<ScriptableObject*>(new T()));
			return script;
		}

		template<typename T>
		T* GetInstance()
		{
			for (auto script : m_Scripts)
			{
				if (typeid(script).name() == typeid(T).name())
				{
					return dynamic_cast<T*>(script);
				}
			}
			return nullptr;
		}

		template<typename T>
		void RemoveInstance()
		{
			int index = -1;
			for (auto script : m_Scripts)
			{
				index++;
				if (typeid(script).name() == typeid(T).name())
				{
					m_Scripts.erase(m_Scripts.begin() + index);
					break;
				}
			}
		}
	public:
		ScriptableObject* Instance = nullptr;
		std::vector<ScriptableObject*>		m_Scripts;
	};
}



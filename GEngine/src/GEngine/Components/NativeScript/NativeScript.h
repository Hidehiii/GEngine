#pragma once
#include "GEngine/Core/Core.h"
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

		template<typename T>
		void AddInstance()
		{
			GE_CORE_ASSERT(HasInstance<T>() == false, "There is a same script has been added!");
			m_TempAddScripts.emplace_back(nullptr, []() { return static_cast<ScriptableObject*>(new T()); });
			//m_Scripts.emplace_back(nullptr, []() { return static_cast<ScriptableObject*>(new T()); });
		}

		template<typename T>
		T* GetInstance()
		{
			for (auto script : m_Scripts)
			{
				if (script.first == nullptr)
				{
					GE_CORE_INFO("nullptr script found!");
					continue;
				}
				if (typeid(*(script.first)).name() == typeid(T).name())
				{
					return dynamic_cast<T*>(script.first);
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
				if (typeid(*script.first).name() == typeid(T).name())
				{
					m_TempRemoveScripts.push_back(script.first);
					//m_Scripts.erase(m_Scripts.begin() + index);
					break;
				}
			}
		}

		template<typename T>
		bool HasInstance()
		{
			for (auto script : m_Scripts)
			{
				if (typeid(*(script.second())).name() == typeid(T).name())
				{
					return true;
				}
			}
			return false;
		}

		void Excute()
		{
			for (auto& removeScript : m_TempRemoveScripts)
			{
				for (int i = 0; i < m_Scripts.size(); i++)
				{
					if (m_Scripts[i].first == removeScript)
					{
						delete m_Scripts[i].first;
						m_Scripts[i].first = nullptr;
						m_Scripts.erase(m_Scripts.begin() + i);
						break;
					}
				}
			}
			m_TempRemoveScripts.clear();
			for (int i = 0; i < m_TempAddScripts.size(); i++)
			{
				m_Scripts.emplace(m_Scripts.begin(), m_TempAddScripts[i]);
			}
			m_TempAddScripts.clear();
		}
	public:
		std::vector<std::pair<ScriptableObject*, ScriptableObject* (*)()>>		m_Scripts;
	private:
		std::vector<std::pair<ScriptableObject*, ScriptableObject* (*)()>>		m_TempAddScripts;
		std::vector<ScriptableObject*>											m_TempRemoveScripts;
	};
}



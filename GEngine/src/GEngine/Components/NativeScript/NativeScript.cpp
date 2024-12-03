#include "GEpch.h"
#include "NativeScript.h"

namespace GEngine
{
    void NativeScript::OnRender()
    {
		for (auto& script : m_Scripts)
		{
			if (script.first != nullptr)
			{
				script.first->OnRender();
			}
		}
    }
    void NativeScript::OnAwake()
    {
        Excute();
        for (auto& script : m_Scripts)
        {
            if (script.first == nullptr)
            {
                script.first = script.second();
                script.first->m_GameObject = m_GameObject;
            }
            script.first->OnAwake();
        }
        Excute();
    }
    void NativeScript::OnStart()
    {
		for (auto& script : m_Scripts)
		{
			if (script.first == nullptr)
			{
				script.first = script.second();
				script.first->m_GameObject = m_GameObject;
                script.first->OnAwake();
			}
            script.first->OnStart();
		}
		Excute();
    }
    void NativeScript::OnUpdate()
    {
		for (auto& script : m_Scripts)
		{
			if (script.first == nullptr)
			{
				script.first = script.second();
				script.first->m_GameObject = m_GameObject;
				script.first->OnAwake();
				script.first->OnStart();
			}
            script.first->OnUpdate();
		}
		Excute();
    }
    void NativeScript::OnPhysicsUpdate()
    {
		for (auto& script : m_Scripts)
		{
			if (script.first == nullptr)
			{
				script.first = script.second();
				script.first->m_GameObject = m_GameObject;
				script.first->OnAwake();
                script.first->OnStart();
			}
            script.first->OnPhysicsUpdate();
		}
		Excute();
    }
    void NativeScript::OnLateUpdate()
    {
		for (auto& script : m_Scripts)
		{
			if (script.first == nullptr)
			{
				script.first = script.second();
				script.first->m_GameObject = m_GameObject;
				script.first->OnAwake();
				script.first->OnStart();
			}
			script.first->OnLateUpdate();
		}
		Excute();
    }
    void NativeScript::OnDestroy()
    {
		for (auto& script : m_Scripts)
		{
			if (script.first == nullptr)
			{
				script.first = script.second();
				script.first->m_GameObject = m_GameObject;
				script.first->OnAwake();
				script.first->OnStart();
			}
			script.first->OnDestroy();
			delete script.first;
			script.first = nullptr;
		}
    }
}
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
	public:
		ScriptableObject* Instance = nullptr;
		std::vector<ScriptableObject*>		m_Scripts;
	};
}



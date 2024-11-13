#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Events/Event.h"


namespace GEngine
{
	class GENGINE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}
		virtual void OnRender() {}
		virtual void OnLateUpdate() {}
		virtual void OnPresent() {}
		virtual void OnEndFrame() {}
		

		inline const std::string& GetName() const { return m_Name; }

	protected:
		std::string m_Name;
	};
}



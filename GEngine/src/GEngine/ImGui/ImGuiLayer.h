#pragma once

#include "GEngine/Layer.h"
#include "ImGui/imgui.h"
#include <ImGuizmo.h>
#include "PlatformImGui.h"
#include "GEngine/Renderer/Texture.h"

namespace GEngine
{

	

	class GENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override; 

		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
		virtual void Begin();
		virtual void End();

		void SetBlockEvent(bool block) { m_BlockEvents = block; }
		void SetDarkThemeColor();
		ImGuiContext* GetContext() { return m_Context; }
		Ref<Texture2D>	GetImGuiImage() { return m_PlatformImGui->GetImGuiTexture(); }

		virtual void AddWaitCommand(Ref<CommandBuffer> cmd) { m_PlatformImGui->AddWaitCommand(cmd); };
		virtual void ClearWaitCommands() { m_PlatformImGui->ClearWaitCommands(); };
	private:
		bool			m_BlockEvents = true;
		float			m_Time = 0.0f;
		ImGuiContext*	m_Context;
		PlatformImGui*	m_PlatformImGui = nullptr;
	};
}



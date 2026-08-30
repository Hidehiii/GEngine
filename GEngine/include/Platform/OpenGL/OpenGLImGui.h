#pragma once

#include "GEngine/ImGui/PlatformImGui.h"

namespace GEngine
{
	class GENGINE_API OpenGLImGui : public PlatformImGui
	{
	public:
		OpenGLImGui() {}
		virtual ~OpenGLImGui();
		virtual void OnAttach(void* window) override;

		virtual void Begin() override;
		virtual void End() override;

		virtual Ref<Texture2D> GetImGuiTexture() override;

		virtual Ref<CommandBuffer> GetCommandBuffer() override;
	};
}



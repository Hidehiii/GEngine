#pragma once
#include "GEngine/ImGui/PlatformImGui.h"

namespace GEngine
{
	class GENGINE_API D3D12ImGui : public PlatformImGui
	{
	public:
		D3D12ImGui() {}
		virtual ~D3D12ImGui();

		virtual void OnAttach(void* window) override;

		virtual void Begin() override;
		virtual void End() override;

		virtual Ref<Texture2D> GetImGuiTexture() override;

		virtual Ref<CommandBuffer> GetCommandBuffer() override;
	};
}



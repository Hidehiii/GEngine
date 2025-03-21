#pragma once
#include "GEngine/ImGui/PlatformImGui.h"

#include "GEngine/Math/Math.h"

class GLFWwindow;
namespace GEngine
{
	
	class VulkanTexture2D;
	class VulkanCommandBuffer;

	class GENGINE_API VulkanImGui : public PlatformImGui
	{
	public:
		VulkanImGui() {}
		virtual void OnAttach(GLFWwindow* window) override;
		virtual void OnDetach() override;

		virtual void Begin() override;
		virtual void End() override;

		virtual Ref<Texture2D> GetImGuiTexture() override;

		virtual Ref<CommandBuffer> GetCommandBuffer() override;
	private:
		void CreateBuffer();
		void CreateCommandBufferAndSyncObjects();
	private:
		
	};

}


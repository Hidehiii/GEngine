#pragma once
#include "GEngine/ImGui/PlatformImGui.h"



class GLFWwindow;
namespace GEngine
{
	
	class GENGINE_API VulkanImGui : public PlatformImGui
	{
	public:
		VulkanImGui() {}
		virtual void OnAttach(GLFWwindow* window) override;
		virtual void OnDetach() override;

		virtual void Begin() override;
		virtual void End() override;

		virtual Ref<Texture2D> GetImGuiTexture() override;
	private:
		void CreateBuffer();
	};

}


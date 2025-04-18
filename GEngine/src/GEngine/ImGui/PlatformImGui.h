#pragma once
#include "ImGui/imgui.h"
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Texture.h"

class GLFWwindow;
namespace GEngine
{

	class CommandBuffer;

	class GENGINE_API PlatformImGui
	{
	public:
		PlatformImGui() {};
		virtual void OnAttach(GLFWwindow* window) = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual Ref<Texture2D> GetImGuiTexture() = 0;

		virtual Ref<CommandBuffer> GetCommandBuffer() = 0;
	};
}
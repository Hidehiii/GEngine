#include "GEpch.h"
#include "GEngine/Utils/GUIUtils.h"
#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_vulkan.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

namespace GEngine
{
	void* GUIUtils::GetTextureID(const Ref<Texture2D>& texture)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return (void*)std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetRendererID();
		case RendererAPI::API::Vulkan:
			return ImGui_ImplVulkan_AddTexture(std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetSampler(), 
				std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImageView(), 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		default:
			break;
		}

		GE_CORE_ASSERT(false, "GetTextureID not work");
		return nullptr;
	}
}
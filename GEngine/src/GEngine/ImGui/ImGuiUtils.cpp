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
	std::vector < Ref<Texture2D>> s_VulkanTexture2DUsed;
	std::vector< VkDescriptorSet> s_VulkanTextureID;
	void* GUIUtils::GetTextureID(const Ref<Texture2D>& texture)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return (void*)std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetRendererID();
		case RendererAPI::API::Vulkan:
		{
			for (int i = s_VulkanTexture2DUsed.size() - 1; i >= 0; i--)
			{
				if (s_VulkanTexture2DUsed.at(i).use_count() <= 1)
				{
					ImGui_ImplVulkan_RemoveTexture(s_VulkanTextureID.at(i));
					s_VulkanTexture2DUsed.erase(s_VulkanTexture2DUsed.begin() + i);
					s_VulkanTextureID.erase(s_VulkanTextureID.begin() + i);
				}
			}
			for (int i = 0; i < s_VulkanTexture2DUsed.size(); i++)
			{
				if (s_VulkanTexture2DUsed.at(i) == texture)
				{
					return s_VulkanTextureID.at(i);
				}
			}
			VkDescriptorSet dst = ImGui_ImplVulkan_AddTexture(std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetSampler(),
				std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImageView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			s_VulkanTexture2DUsed.push_back(texture);
			s_VulkanTextureID.push_back(dst);
			return dst;
		}
			
		default:
			break;
		}

		GE_CORE_ASSERT(false, "GetTextureID not work");
		return nullptr;
	}
}
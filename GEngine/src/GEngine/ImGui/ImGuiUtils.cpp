#include "GEpch.h"
#include "GEngine/Utils/GUIUtils.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanSampler.h"

namespace GEngine
{
	std::vector < Ref<Texture2D>> s_VulkanTexture2DUsed;
	std::vector< VkDescriptorSet> s_VulkanTextureID;
	uint64_t GUIUtils::GetTextureID(const Ref<Texture2D>& texture)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:
			return std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetOpenGLID();
		case GRAPHICS_API_VULKAN:
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
					return reinterpret_cast<uint64_t>(s_VulkanTextureID.at(i));
				}
			}
			VkDescriptorSet dst = ImGui_ImplVulkan_AddTexture(std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImageView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			s_VulkanTexture2DUsed.push_back(texture);
			s_VulkanTextureID.push_back(dst);
			return reinterpret_cast<uint64_t>(dst);
		}
			
		default:
			break;
		}

		throw std::runtime_error("GUI texture IDs are unsupported by this graphics backend");
	}

	Vector2 GUIUtils::GetUV0()
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:
			return { 0.0f, 1.0f };
		case GRAPHICS_API_VULKAN:
			return { 0.0f, 0.0f };

		default:
			break;
		}

		GE_CORE_ASSERT(false, "GetUV0 not work");
		return Vector2();
	}

	Vector2 GUIUtils::GetUV1()
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:
			return { 1.0f, 0.0f };
		case GRAPHICS_API_VULKAN:
			return { 1.0f, 1.0f };

		default:
			break;
		}

		GE_CORE_ASSERT(false, "GetUV1 not work");
		return Vector2();
	}
}

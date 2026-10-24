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
#include "Platform/D3D12/D3D12ImGui.h"

namespace GEngine
{
    struct UIFrameTextures
    {
        std::vector<Ref<Texture2D>> Textures;
        std::vector<VkDescriptorSet> Descriptors;
    };
    static std::vector<UIFrameTextures> s_TextureFrames;
    void BeginImGuiTextureFrame()
    {
        s_TextureFrames.resize(Graphics::GetFrameCount());
        auto& frame = s_TextureFrames.at(Graphics::GetFrame());
        for (auto descriptor : frame.Descriptors) ImGui_ImplVulkan_RemoveTexture(descriptor);
        frame.Descriptors.clear();
        frame.Textures.clear();
    }
    void ShutdownImGuiTextures()
    {
        for (auto& frame : s_TextureFrames)
            for (auto descriptor : frame.Descriptors) ImGui_ImplVulkan_RemoveTexture(descriptor);
        s_TextureFrames.clear();
    }
	uint64_t GUIUtils::GetTextureID(const Ref<Texture2D>& texture)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL:
			return std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetOpenGLID();
		case GRAPHICS_API_DIRECT3DX12:
			return D3D12ImGui::RegisterTexture(texture);
		case GRAPHICS_API_VULKAN:
		{
            auto& frame = s_TextureFrames.at(Graphics::GetFrame());
            auto& s_VulkanTexture2DUsed = frame.Textures;
            auto& s_VulkanTextureID = frame.Descriptors;
            for (size_t i = 0; i < frame.Textures.size(); ++i)
                if (frame.Textures[i] == texture) return reinterpret_cast<uint64_t>(frame.Descriptors[i]);
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
		case GRAPHICS_API_DIRECT3DX12:
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
		case GRAPHICS_API_DIRECT3DX12:
			return { 1.0f, 1.0f };

		default:
			break;
		}

		GE_CORE_ASSERT(false, "GetUV1 not work");
		return Vector2();
	}
}

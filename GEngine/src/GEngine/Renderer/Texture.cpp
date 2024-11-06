#include "GEpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace GEngine
{
	// vulkan glsl 中的layout(binding)要用
	uint32_t Texture2D::s_Texture2DBindingOffsetForVulkan = 10;

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLTexture2D>(width, height);
		}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanTexture2D>(width, height);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLTexture2D>(path);
		}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanTexture2D>(path);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::WhiteTexture()
	{
		Ref<Texture2D> texture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		texture->SetData(&whiteTextureData, sizeof(uint32_t));
		return texture;
	}
}
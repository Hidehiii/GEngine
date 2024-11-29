#include "GEpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace GEngine
{
	Ref<Texture2D>	Texture2D::s_WhiteTexture2D = nullptr;

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLTexture2D>(width, height, format);
		}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanTexture2D>(width, height, format);
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
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLTexture2D>(width, height, data, size, format);
		}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanTexture2D>(width, height, data, size, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::White()
	{
		if (s_WhiteTexture2D == nullptr)
		{
			uint32_t		whiteTexture2DData = 0xffffffff;
			s_WhiteTexture2D = Texture2D::Create(1, 1, &whiteTexture2DData, sizeof(uint32_t));
		}
		return s_WhiteTexture2D;
	}
}
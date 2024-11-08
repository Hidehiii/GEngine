#include "GEpch.h"
#include "GEngine/Utils/GUIUtils.h"
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
			GE_CORE_ASSERT(false, "Vulkan not supported yet");
		default:
			break;
		}

		GE_CORE_ASSERT(false, "GetTextureID not work");
		return nullptr;
	}
}
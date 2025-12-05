#include "GEpch.h"
#include "Graphics.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/UniformBuffer.h"
#include "GEngine/Components/Components.h"
#include "Platform/OpenGL/OpenGLGraphicsAPI.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"
#include "GEngine/Tools/ShaderCompiler.h"

namespace GEngine
{
	GraphicsAPI*				Graphics::s_GraphicsAPI				= nullptr;
	uint8_t						Graphics::s_FrameCount				= 0;
	uint8_t						Graphics::s_Frame					= 0;
	uint32_t					Graphics::s_CommandBufferCount		= 1000;
	uint32_t					Graphics::s_ViewportWidth			= 0;
	uint32_t					Graphics::s_ViewportHeight			= 0;
	bool						Graphics::s_ReverseDepth			= false;

	void Graphics::Setup(const GraphicsSpecification& spec)
	{
		switch (spec.API)
		{
		case GRAPHICS_API_NONE:
		case GRAPHICS_API_DIRECT3DX12:
			GE_CORE_ASSERT(false, "Renderer api is not supported");
			break;
		case GRAPHICS_API_OPENGL:
			s_GraphicsAPI = new OpenGLGraphicsAPI();
			break;
		case GRAPHICS_API_VULKAN:
			s_GraphicsAPI = new VulkanGraphicsAPI();
		default:
			break;
		}

		s_Frame					= 0;
		s_FrameCount			= spec.FramesInFlight;
		s_CommandBufferCount	= spec.CommandBufferCount;
		s_ViewportWidth			= spec.ViewportWidth;
		s_ViewportHeight		= spec.ViewportHeight;
	}

	void Graphics::Init()
    {
		ShaderCompiler::Create();
    }
	void Graphics::FrameMove()
	{
		s_Frame = s_Frame + 1 < s_FrameCount ? s_Frame + 1 : 0;
	}
	void Graphics::SetViewport(uint32_t width, uint32_t height)
	{
		s_ViewportWidth = width;
		s_ViewportHeight = height;
	}
	void Graphics::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
	{
		s_GraphicsAPI->SetCommandsBarrier(first, second);
	}
	void Graphics::SetReverseDepth(bool reverse)
	{
		s_ReverseDepth = reverse;
	}
	Graphics_API Graphics::GetGraphicsAPI()
	{
		return s_GraphicsAPI->GetAPI();
	}
	uint8_t Graphics::GetFrameCount()
	{
		return s_FrameCount;
	}
	uint8_t Graphics::GetFrame()
	{
		return s_Frame;
	}
	uint32_t Graphics::GetCommandBufferCount()
	{
		return s_CommandBufferCount;
	}
	bool Graphics::IsReverseDepth()
	{
		return s_ReverseDepth;
	}
	Ref<CommandBuffer> Graphics::GetGraphicsCommandBuffer()
	{
		return s_GraphicsAPI->GetGraphicsCommandBuffer();
	}
	Ref<CommandBuffer> Graphics::GetComputeCommandBuffer()
	{
		return s_GraphicsAPI->GetComputeCommandBuffer();
	}
	uint32_t Graphics::GetMaxTextureSize()
	{
		return s_GraphicsAPI->GetMaxTextureSize();
	}
	uint32_t Graphics::GetMaxCombinedTextureCount()
	{
		return s_GraphicsAPI->GetMaxCombinedTextureCount();
	}
	uint32_t Graphics::GetMaxPerStageTextureCount()
	{
		return s_GraphicsAPI->GetMaxPerStageTextureCount();
	}
	uint32_t Graphics::GetMaxTextureArrayLayers()
	{
		return s_GraphicsAPI->GetMaxTextureArrayLayers();
	}
	uint32_t Graphics::GetMinUniformBufferOffsetAlignment()
	{
		return s_GraphicsAPI->GetMinUniformBufferOffsetAlignment();
	}
	uint32_t Graphics::GetMaxUniformBufferSize()
	{
		return s_GraphicsAPI->GetMaxUniformBufferSize();
	}
	Vector3 Graphics::GetMaxComputeWorkGroupCount()
	{
		return s_GraphicsAPI->GetMaxComputeWorkGroupCount();
	}
	Vector3 Graphics::GetMaxComputeWorkGroupSize()
	{
		return s_GraphicsAPI->GetMaxComputeWorkGroupSize();
	}
	uint32_t Graphics::GetMaxComputeWorkGroupInvocations()
	{
		return s_GraphicsAPI->GetMaxComputeWorkGroupInvocations();
	}
	uint32_t Graphics::GetViewportWidth()
	{
		return s_ViewportWidth;
	}
	uint32_t Graphics::GetViewportHeight()
	{
		return s_ViewportHeight;
	}

}
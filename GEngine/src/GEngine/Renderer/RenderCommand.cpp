#include "GEpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace GEngine
{
	//RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();

	inline void RenderCommand::Init()
	{
		s_RendererAPI->Init();
	}

	inline void RenderCommand::SetClearColor(const Vector4& color)
	{
		s_RendererAPI->SetClearColor(color);
	}

	inline void RenderCommand::Clear()
	{
		s_RendererAPI->Clear();
	}

	inline void RenderCommand::DrawTriangles(const uint32_t indexcount)
	{
		s_RendererAPI->DrawTriangles(indexcount);
	}
	inline void RenderCommand::DrawTriangleInstance(const uint32_t indexcount, const uint32_t instanceCount)
	{
		s_RendererAPI->DrawTrianglesInstance(indexcount, instanceCount);
	}
	inline void RenderCommand::DrawLines(const uint32_t indexcount)
	{
		s_RendererAPI->DrawLines(indexcount);
	}
	inline void RenderCommand::DrawPoints(const uint32_t indexcount)
	{
		s_RendererAPI->DrawPoints(indexcount);
	}
	inline void RenderCommand::SetLineWidth(float width)
	{
		s_RendererAPI->SetLineWidth(width);
	}
	inline void RenderCommand::SetPointSize(float size)
	{
		s_RendererAPI->SetPointSize(size);
	}
	inline void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		s_RendererAPI->SetViewport(x, y, width, height);
	}
	inline float RenderCommand::GetTime()
	{
		return s_RendererAPI->GetTime();
	}
	inline void RenderCommand::SetCull(CullMode mode)
	{
		s_RendererAPI->SetCull(mode);
	}
	inline void RenderCommand::SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
	{
		s_RendererAPI->SetBlend(modeColor, modeAlpha, srcColor, dstColor, srcAlpha, dstAlpha);
	}
	inline void RenderCommand::EnableDepthWrite(bool enabled)
	{
		s_RendererAPI->EnableDepthWrite(enabled);
	}
	inline void RenderCommand::SetDepthTest(CompareOperation op)
	{
		s_RendererAPI->SetDepthTest(op);
	}
	inline void RenderCommand::BeginGraphicsCommand()
	{
		s_RendererAPI->BeginGraphicsCommand();
	}
	inline void RenderCommand::EndGraphicsCommand()
	{
		s_RendererAPI->EndGraphicsCommand();
	}
	inline void RenderCommand::BeginSecondaryCommand()
	{
		s_RendererAPI->BeginSecondaryCommand();
	}
	inline void RenderCommand::EndSecondaryCommand()
	{
		s_RendererAPI->EndSecondaryCommand();
	}
	inline void RenderCommand::DrawTriangles(CommandBuffer* buffer, const uint32_t indexcount)
	{
		s_RendererAPI->DrawTriangles(buffer, indexcount);
	}
	inline void RenderCommand::DrawTriangleInstance(CommandBuffer* buffer, const uint32_t indexcount, const uint32_t instanceCount)
	{
		s_RendererAPI->DrawTrianglesInstance(buffer, indexcount, instanceCount);
	}
	inline void RenderCommand::DrawLines(CommandBuffer* buffer, const uint32_t indexcount)
	{
		s_RendererAPI->DrawLines(buffer, indexcount);
	}
	inline void RenderCommand::DrawPoints(CommandBuffer* buffer, const uint32_t indexcount)
	{
		s_RendererAPI->DrawPoints(buffer, indexcount);
	}
	CommandBuffer* RenderCommand::BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Camera& camera)
	{
		return s_RendererAPI->BeginGraphicsCommand(buffer, camera);
	}
	CommandBuffer* RenderCommand::BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera)
	{
		return s_RendererAPI->BeginGraphicsCommand(buffer, camera);
	}
	void RenderCommand::EndGraphicsCommand(CommandBuffer* buffer)
	{
		s_RendererAPI->EndGraphicsCommand(buffer);
	}
	inline void RenderCommand::SetRendererAPI(RendererAPI::API api)
	{
		switch (api)
		{
		case GEngine::RendererAPI::API::None:
			GE_CORE_ASSERT(false, "Renderer api is not supported");
			break;
		case GEngine::RendererAPI::API::OpenGL:
			s_RendererAPI = new OpenGLRendererAPI();
			break;
		case GEngine::RendererAPI::API::Vulkan:
			s_RendererAPI = new VulkanRendererAPI();
		default:
			break;
		}
	}
	inline std::vector<std::string> RenderCommand::GetExtensions()
	{
		return s_RendererAPI->GetExtensions();
	}
	inline uint32_t RenderCommand::GetMaxTextureSize()
	{
		return s_RendererAPI->GetMaxTextureSize();
	}
	inline uint32_t RenderCommand::GetMaxCombinedTextureCount()
	{
		return s_RendererAPI->GetMaxCombinedTextureCount();
	}
	inline uint32_t RenderCommand::GetMaxPerStageTextureCount()
	{
		return s_RendererAPI->GetMaxPerStageTextureCount();
	}
	inline uint32_t RenderCommand::GetMaxTextureArrayLayers()
	{
		return s_RendererAPI->GetMaxTextureArrayLayers();
	}
	inline uint32_t RenderCommand::GetMinUniformBufferOffsetAlignment()
	{
		return s_RendererAPI->GetMinUniformBufferOffsetAlignment();
	}
	inline uint32_t RenderCommand::GetMaxUniformBufferSize()
	{
		return s_RendererAPI->GetMaxUniformBufferSize();
	}
	inline Vector3 RenderCommand::GetMaxComputeWorkGroupCount()
	{
		return s_RendererAPI->GetMaxComputeWorkGroupCount();
	}
	inline Vector3 RenderCommand::GetMaxComputeWorkGroupSize()
	{
		return s_RendererAPI->GetMaxComputeWorkGroupSize();
	}
	inline uint32_t RenderCommand::GetMaxComputeWorkGroupInvocations()
	{
		return s_RendererAPI->GetMaxComputeWorkGroupInvocations();
	}
	inline void RenderCommand::BeginComputeCommand()
	{
		s_RendererAPI->BeginComputeCommand();
	}
	inline void RenderCommand::EndComputeCommand()
	{
		s_RendererAPI->EndComputeCommand();
	}
	inline void RenderCommand::Compute(const uint32_t x, const uint32_t y, const uint32_t z)
	{
		s_RendererAPI->Compute(x, y, z);
	}
}

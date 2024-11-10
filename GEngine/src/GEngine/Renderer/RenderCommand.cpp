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

	inline void RenderCommand::DrawTriangles(const Ref<VertexBuffer>& vertexBuffer, const uint32_t count = 0)
	{
		s_RendererAPI->DrawTriangles(vertexBuffer, count);
	}
	inline void RenderCommand::DrawTriangles(const Ref<VertexBuffer>& vertexBuffer)
	{
		s_RendererAPI->DrawTriangles(vertexBuffer);
	}
	inline void RenderCommand::DrawLines(const Ref<VertexBuffer>& vertexBuffer, const uint32_t count)
	{
		s_RendererAPI->DrawLines(vertexBuffer, count);
	}
	inline void RenderCommand::DrawPoints(const Ref<VertexBuffer>& vertexBuffer, const uint32_t count)
	{
		s_RendererAPI->DrawPoints(vertexBuffer, count);
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
	inline void RenderCommand::SetCull(MaterialCullMode mode)
	{
		s_RendererAPI->SetCull(mode);
	}
	inline void RenderCommand::SetBlend(MaterialBlendMode mode, uint32_t source, uint32_t dest)
	{
		s_RendererAPI->SetBlend(mode, source, dest);
	}
	inline void RenderCommand::EnableDepthWrite(bool enabled)
	{
		s_RendererAPI->EnableDepthWrite(enabled);
	}
	inline void RenderCommand::EnableDepthTest(bool enabled)
	{
		s_RendererAPI->EnableDepthTest(enabled);
	}
	inline void RenderCommand::BeginDrawCommand()
	{
		s_RendererAPI->BeginDrawCommand();
	}
	inline void RenderCommand::EndDrawCommand()
	{
		s_RendererAPI->EndDrawCommand();
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
}

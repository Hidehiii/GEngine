#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"

namespace GEngine
{
	class GENGINE_API RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = default;

		virtual std::vector<Ref<RenderPass>>	GetRenderPasses() { return m_RenderPasses; }
		virtual std::vector<Ref<FrameBuffer>>	GetFrameBuffers() { return m_FrameBuffers; }
		virtual Ref<RenderPass>					GetRenderPass(int index) { return m_RenderPasses.at(index); }
		virtual Ref<FrameBuffer>				GetFrameBuffer(int index) { return m_FrameBuffers.at(index); }

		// 创建pass和frmaebuffer
		virtual void Init() = 0;
		// 自定义渲染流程
		virtual void Render() = 0;

	protected:
		std::vector<Ref<RenderPass>>	m_RenderPasses;
		std::vector<Ref<FrameBuffer>>	m_FrameBuffers;
	};
}



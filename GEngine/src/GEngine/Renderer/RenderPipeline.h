#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"

namespace GEngine
{
	class GENGINE_API RenderPipeline
	{
	public:
		// 继承后需要在构造函数里创建pass和framebuffer
		virtual ~RenderPipeline() = default;

		virtual std::vector<Ref<FrameBuffer>>	GetFrameBuffers() { return m_FrameBuffers; }
		virtual Ref<FrameBuffer>				GetFrameBuffer(int index) { return m_FrameBuffers.at(index); }

		// 自定义渲染流程
		virtual void Render() = 0;

	protected:
		std::vector<Ref<FrameBuffer>>	m_FrameBuffers;
	};
}



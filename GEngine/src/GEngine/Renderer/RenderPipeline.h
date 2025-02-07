#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"

namespace GEngine
{
	class GENGINE_API RenderPipeline
	{
	public:
		// �̳к���Ҫ�ڹ��캯���ﴴ��pass��framebuffer
		virtual ~RenderPipeline() = default;

		virtual std::vector<Ref<FrameBuffer>>	GetFrameBuffers() { return m_FrameBuffers; }
		virtual Ref<FrameBuffer>				GetFrameBuffer(int index) { return m_FrameBuffers.at(index); }

		// �Զ�����Ⱦ����
		virtual void Render() = 0;

	protected:
		std::vector<Ref<FrameBuffer>>	m_FrameBuffers;
	};
}



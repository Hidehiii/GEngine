#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/FrameBuffer.h"

namespace GEngine
{
	class GENGINE_API RenderPipeline
	{
	public:
		// �̳к���Ҫ�ڹ��캯���ﴴ��pass��framebuffer
		virtual ~RenderPipeline() = default;

		// �Զ�����Ⱦ����
		virtual void Render() = 0;

		// �Զ����������
		virtual void Present() = 0;

	protected:
		// frame buffer �����ÿ����Ļ��Сȷ��
		std::vector<Ref<FrameBuffer>>	m_FrameBuffers;
	};
}



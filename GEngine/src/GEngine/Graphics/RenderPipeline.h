#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Components/Camera/Camera.h"

namespace GEngine
{
	class Camera;

	class GENGINE_API RenderPipeline
	{
	public:
		// �̳к���Ҫ�ڹ��캯���ﴴ��pass
		virtual ~RenderPipeline() = default;

		// �Զ�����Ⱦ����
		virtual void Render() = 0;

		// �Զ����������
		virtual void Present() = 0;

	protected:
		// ��˳��ִ������pass
		std::vector<Ref<RenderPass>>	m_RenderPasses;
		// frame buffer �����ÿ����Ļ��Сȷ��
		std::vector<Ref<FrameBuffer>>	m_FrameBuffers;
	};
}



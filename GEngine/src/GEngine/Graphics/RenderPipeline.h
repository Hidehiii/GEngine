#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Components/Camera/Camera.h"

namespace GEngine
{
	class GENGINE_API RenderPipeline
	{
	public:
		// �̳к���Ҫ�ڹ��캯���ﴴ��pass��framebuffer
		virtual ~RenderPipeline() = default;

		// �Զ�����Ⱦ����
		virtual void Render(std::vector<Camera>& cameras) = 0;

		// �Զ����������
		virtual void Present(std::vector<Camera>& cameras) = 0;
	};
}



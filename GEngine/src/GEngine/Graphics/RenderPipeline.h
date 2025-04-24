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
		// 继承后需要在构造函数里创建pass
		virtual ~RenderPipeline() = default;

		// 自定义渲染流程
		virtual void Render() = 0;

		// 自定义最终输出
		virtual void Present() = 0;

	protected:
		// 按顺序执行所有pass
		std::vector<Ref<RenderPass>>	m_RenderPasses;
		// frame buffer 会根据每个屏幕大小确定
		std::vector<Ref<FrameBuffer>>	m_FrameBuffers;
	};
}



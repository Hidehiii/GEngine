#pragma once

#include <GEngine.h>

namespace GEngine
{
	// A small visual smoke test for the portable RenderSystem/RenderGraph path.
	// No backend header or native resource handle is used by this layer.
	class FrameGraphTriangleLayer final : public Layer
	{
	public:
		FrameGraphTriangleLayer();
		void OnAttach() override;
		void OnPresent() override;

	private:
		Ref<GraphicsPipeline> m_Pipeline;
	};
}

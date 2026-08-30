#pragma once

#include <GEngine.h>

namespace GEngine
{
	class TriangleLayer final : public Layer
	{
	public:
		TriangleLayer();
		void OnAttach() override;
		void OnPresent() override;

	private:
		Ref<GraphicsPipeline> m_Pipeline;
	};
}

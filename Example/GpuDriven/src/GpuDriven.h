#pragma once
#include <GEngine.h>

#define TRIANGLES_COUNT_PER_CLUSTER 64

namespace GEngine
{
	

	struct PresentVertex
	{
		Vector4 Position;
		Vector2 UV;
	};

	class GpuDriven :public Layer
	{
	public:
		GpuDriven();
		virtual ~GpuDriven() = default;

		virtual void OnAttach() override;
		virtual void OnPresent() override;
		virtual void OnRender() override;

		void OnUpdate() override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		Editor::EditorCamera m_EditorCamera;
	};
}



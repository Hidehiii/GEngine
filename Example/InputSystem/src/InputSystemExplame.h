#pragma once
#include <GEngine.h>

namespace GEngine
{
	class InputSystemExplame : public Layer
	{
	public:
		InputSystemExplame();
		virtual ~InputSystemExplame() = default;

		virtual void OnAttach() override;
		virtual void OnPresent() override;
		virtual void OnRender() override;
		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	};
}


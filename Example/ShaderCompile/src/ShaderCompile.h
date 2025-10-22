#pragma once
#include <GEngine.h>

namespace GEngine
{
	class ShaderCompile : public Layer
	{
	public:
		ShaderCompile();
		virtual ~ShaderCompile() = default;

		virtual void OnAttach() override;
		virtual void OnPresent() override;
		virtual void OnRender() override;
		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	};
}



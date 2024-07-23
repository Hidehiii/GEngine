#pragma once

#include <GEngine.h>
#include "RayTracing.h"


class Sandbox2D : public GEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate() override;
	virtual void OnGuiRender() override;
	void OnEvent(GEngine::Event& e) override;
private:
	GEngine::Editor::EditorCamera m_Camera;
	GEngine::Editor::EditorCamera m_EditorCamera;
	RayTracing m_RayTracing;
};


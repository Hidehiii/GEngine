#pragma once

#include <GEngine.h>

#include "RayTracing.h"

using namespace GEngine;

struct TestVertex
{
	Vector4	Pos;
	Vector2 UV;
	int		index;
};

struct PresentVertex
{
	Vector4 Pos;
	Vector2 UV;
};


class Sandbox2D : public GEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnPresent() override;
	virtual void OnRender() override;

	void OnUpdate() override;
	virtual void OnGuiRender() override;
	void OnEvent(GEngine::Event& e) override;
private:
	bool OnKeyPressed(GEngine::KeyPressedEvent& e);
private:
	/*GEngine::Editor::EditorCamera m_Camera;
	GEngine::Editor::EditorCamera m_EditorCamera;
	RayTracing m_RayTracing;*/

	Ref<Pipeline>	m_Pipeline;
	std::vector<TestVertex> m_vertex;
	Ref<FrameBuffer> m_FrameBuffer;
	Ref<Texture2D> m_Texture;

	std::vector<PresentVertex> m_PresentVertex;
	Ref<Pipeline> m_PresentPipeline;

	Editor::EditorCamera m_EditorCamera;
};


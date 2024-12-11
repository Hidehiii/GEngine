#pragma once

#include <GEngine.h>

#include "RayTracing.h"

using namespace GEngine;

struct TestVertex
{
	Vector4	Pos;
	Vector4 Color;
};

struct PresentVertex
{
	Vector4 Pos;
	Vector2 UV;
};

struct Node
{
	Vector4 color;
	float depth;
	uint32_t next;
};

struct SBOData
{
	uint32_t count;
	uint32_t maxNodeCount;
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
	virtual void OnImGuiRender() override;
	void OnEvent(GEngine::Event& e) override;
private:
	bool OnKeyPressed(GEngine::KeyPressedEvent& e);
private:
	/*GEngine::Editor::EditorCamera m_Camera;
	GEngine::Editor::EditorCamera m_EditorCamera;
	RayTracing m_RayTracing;*/

	Ref<StorageImage2D> m_StorageImage;
	Ref<StorageBuffer>	m_StorageBuffer;
	Ref<StorageBuffer>	m_SBO;

	Ref<Pipeline> m_OITPrepare;
	Ref<Pipeline> m_OIT;

	Ref<FrameBuffer> m_OIT_1;

	std::vector<TestVertex> m_vertex;

	Ref<Texture2D> m_Texture;
	Ref<CubeMap> m_CubeMap;

	std::vector<PresentVertex> m_PresentVertex;
	Ref<Pipeline> m_PresentPipeline;

	Editor::EditorCamera m_EditorCamera;
};


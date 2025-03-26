#pragma once

#include <GEngine.h>

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

	Ref<StorageImage2D> m_ComputeImage2D;

	Ref<GraphicsPipeline> m_OITPrepare;
	Ref<GraphicsPipeline> m_OIT;
	Ref<GraphicsPipeline> m_CopyColorDepth;
	Ref<GraphicsPipeline> m_SeparateTextureSampler;

	Ref<ComputePipeline> m_ComputeTest;
	Ref<GraphicsPipeline> m_ComputeShow;

	Ref<FrameBuffer> m_OIT_Present;
	Ref<FrameBuffer> m_DepthOnly;
	Ref<FrameBuffer> m_SkyBoxFB;

	std::vector<TestVertex> m_vertex;

	Ref<Texture2D> m_Texture;
	Ref<CubeMap> m_CubeMap;

	Ref<Scene> m_Scene;
	ModelImporter m_ModelImporter;
	GameObject m_SkyBox;

	std::vector<PresentVertex> m_PresentVertex;
	Ref<GraphicsPipeline> m_PresentPipeline;

	Editor::EditorCamera m_EditorCamera;
};


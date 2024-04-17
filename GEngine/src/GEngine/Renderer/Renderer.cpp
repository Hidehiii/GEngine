#include "GEpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "GEngine/Core/Time.h"

namespace GEngine
{
	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		GE_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const OrthoGraphicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjection();
	}
	void Renderer::EndScene()
	{
	}
	void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader, const Ref<Transform>& transform)
	{
		shader->Bind();
		shader->SetFloat1("_Time", Time::GetRunTime());
		shader->SetMat4x4("GE_MATRIX_VP", s_SceneData->ViewProjectionMatrix);
		shader->SetMat4x4("GE_MATRIX_M", transform->GetModelMatrix());
		vertexArray->Bind();
		RenderCommand::DrawTriangles(vertexArray);
	}
	void Renderer::SetRenererAPI(RendererAPI::API api)
	{
		RendererAPI::SetAPI(api);
		RenderCommand::SetRendererAPI(api);
	}
}

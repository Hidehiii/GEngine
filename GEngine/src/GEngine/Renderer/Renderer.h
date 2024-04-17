#pragma once
#include "GEngine/Core/Core.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Gengine/Components/Camera/OrthoGraphicCamera.h"
#include "GEngine/Components/Transform/Transform.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{

	class GENGINE_API Renderer
	{
	public:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const OrthoGraphicCamera& camera);
		static void EndScene();

		static void Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader, const Ref<Transform>& transform);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static void SetRenererAPI(RendererAPI::API api);
	private:
		struct SceneData
		{
			Matrix4x4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;
	};
}


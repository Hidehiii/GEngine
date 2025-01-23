#pragma once
#include "GEngine/Core/Core.h"
#include "RenderBuffer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/UniformBuffer.h"
#include "GEngine/Renderer/FrameBuffer.h"

namespace GEngine
{

	class GENGINE_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Editor::EditorCamera& camera);
		static void BeginScene(Camera& camera);
		static void EndScene();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static void SetRenererAPI(RendererAPI::API api);
		static void SetFramesInFlight(uint8_t frames) { s_FramesInFlight = frames; }
		static uint8_t GetFramesInFlight() { return s_FramesInFlight; }
		static uint8_t GetCurrentFrame() { return s_CurrentFrame; }
		static void SetModelUniforms(Transform& transform);
		static void SetLightUniforms(Vector3& main_dir, Vector3& main_color);
		static void SetScreenUniform(Vector4& size);

		static void DrawCubeWireframe(Transform& transform, Vector4 color);
		const static Camera GetRenderTargetCamera() { return s_RenderTargetCamera; }
		const static Vector3 GetRenderTargetCameraPosition() { return s_RenderTargetCameraPosition; }
	private:
		static void SetCameraUniforms(Matrix4x4& v, Matrix4x4& p, Vector3& pos);
		static void SetTimeUniforms();
		static void MoveToNextFrame() { s_CurrentFrame = (s_CurrentFrame + 1) % s_FramesInFlight; }
		static Camera s_RenderTargetCamera;
		static Vector3 s_RenderTargetCameraPosition;
		static uint8_t s_FramesInFlight;
		static uint8_t s_CurrentFrame;
		friend class Application;
	};
}


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
	class CommandBuffer;

	class GENGINE_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(CommandBuffer* cmdBuffer, Vector2 frameBufferSize, const Editor::EditorCamera& camera);
		static void BeginScene(CommandBuffer* cmdBuffer, Vector2 frameBufferSize, Camera& camera);
		static void EndScene(CommandBuffer* cmdBuffer);

		static void BeginScene(const Editor::EditorCamera& camera);
		static void BeginScene(Camera& camera);
		static void EndScene();

		static RendererAPI::API	GetAPI() { return RendererAPI::GetAPI(); }
		const static Camera		GetRenderTargetCamera() { return s_RenderTargetCamera; }
		const static Vector3	GetRenderTargetCameraPosition() { return s_RenderTargetCameraPosition; }
		static uint8_t			GetFramesInFlight() { return s_FramesInFlight; }
		static uint8_t			GetCurrentFrame() { return s_CurrentFrame; }
		static uint32_t			GetCommandBufferCount() { return s_CommandBufferCount; }


		const static std::vector<uint32_t> GetDynamicUniformBufferOffsets() { return s_DynamicUniformBufferOffsets; }

		
		
		static void SetLightUniforms(Vector3& main_dir, Vector3& main_color);
		static void SetScreenUniform(Vector4& size);

		static void DrawCubeWireframe(Transform& transform, Vector4 color);
	private:
		static void SetFramesInFlight(uint8_t frames) { s_FramesInFlight = frames; }
		static void SetRenererAPI(RendererAPI::API api);
		static void SetDynamicUniformBufferSizeScaleFactor(uint32_t factor);
		static void SetCommandBufferCount(uint32_t count);

		static void SetCameraUniforms(Matrix4x4& v, Matrix4x4& p, Vector3& pos);
		static void SetTimeUniforms();
		static void MoveToNextFrame() { s_CurrentFrame = (s_CurrentFrame + 1) % s_FramesInFlight; }
		static void AddDynamicUniformBuffer(Ref<UniformBuffer>& buffer, uint32_t size, uint32_t binding, int& index);
		static void UpdateDynamicUniformOffset(int index);

		// current camera
		static Camera s_RenderTargetCamera;
		static Vector3 s_RenderTargetCameraPosition;

		// frames in flight
		static uint8_t s_FramesInFlight;
		static uint8_t s_CurrentFrame;

		// dynamic ubo
		static uint32_t s_DynamicUniformBufferSizeScaleFactor;
		static std::vector<uint32_t> s_DynamicUniformBufferOffsetIndices;
		static std::vector<uint32_t> s_DynamicUniformBufferOffsets;
		static std::vector<uint32_t> s_DynamicUniformBufferAlignment;

		// public ubo offset index
		static int s_CameraUniformOffsetIndex;
		static int s_TimeUniformOffsetIndex;
		static int s_LightUniformOffsetIndex;
		static int s_ScreenUniformOffsetIndex;

		// cmd buffer count
		static uint32_t s_CommandBufferCount;

		friend class Application;
	};
}


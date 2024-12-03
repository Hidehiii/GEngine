#include "GEpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "GEngine/Core/Time.h"

namespace GEngine
{
	Camera Renderer::s_RenderTargetCamera = Camera();
	struct CameraData
	{
		Matrix4x4 GE_MATRIX_V;
		Matrix4x4 GE_MATRIX_P;
		Matrix4x4 GE_MATRIX_VP;
		Vector4 GE_CAMERA_POSITION;
	};
	struct TimeData
	{
		// x: delta time
		// y: run time
		// z: fixed time
		// w: physical delta time
		Vector4 GE_TIME;
	};

	struct ModelData
	{
		Matrix4x4 GE_MATRIX_M;
	};

	struct LigthData
	{
		Vector4 GE_MAIN_LIGHT_DIRECTION;
		Vector4 GE_MAIN_LIGHT_COLOR;
	};

	struct ScreenData
	{
		// x: width
		// y: height
		Vector4	GE_SCREEN_SIZE;
	};

	struct ShaderUniformData
	{
		CameraData			CameraDataBuffer;
		Ref<UniformBuffer>	CameraUniformBuffer;

		TimeData			TimeDataBuffer;
		Ref<UniformBuffer>	TimeUniformBuffer;

		ModelData			ModelDataBuffer;
		Ref<UniformBuffer>	ModelUniformBuffer;

		LigthData			LigthDataBuffer;
		Ref<UniformBuffer>	LigthUniformBuffer;

		ScreenData			ScreenDataBuffer;
		Ref<UniformBuffer>	ScreenUniformBuffer;
	};

	static ShaderUniformData s_ShaderUniformData;

	void Renderer::Init()
	{
		GE_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();

		// Uniform Buffers的位置是固定的，所以这里直接创建
		// 0 的位置留给自定义
		s_ShaderUniformData.CameraUniformBuffer		= UniformBuffer::Create(sizeof(CameraData), 1);
		s_ShaderUniformData.TimeUniformBuffer		= UniformBuffer::Create(sizeof(TimeData),	2);
		s_ShaderUniformData.ModelUniformBuffer		= UniformBuffer::Create(sizeof(ModelData),	3);
		s_ShaderUniformData.LigthUniformBuffer		= UniformBuffer::Create(sizeof(LigthData),	4);
		s_ShaderUniformData.ScreenUniformBuffer		= UniformBuffer::Create(sizeof(ScreenData), 5);
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const OrthoGraphicCamera& camera)
	{
		SetCameraUniforms(camera.GetView(), camera.GetProjection(), camera.GetPosition());
		SetTimeUniforms();
		SetScreenUniform(Vector4(FrameBuffer::GetCurrentFrameBufferSize().x, FrameBuffer::GetCurrentFrameBufferSize().y, 0, 0));

		Renderer2D::BeginScene();
	}
	void Renderer::BeginScene(const Editor::EditorCamera& camera)
	{
		SetCameraUniforms(camera.GetViewMatrix(), camera.GetProjectionMatrix(), camera.GetPosition());
		SetTimeUniforms();
		SetScreenUniform(Vector4(FrameBuffer::GetCurrentFrameBufferSize().x, FrameBuffer::GetCurrentFrameBufferSize().y, 0, 0));
		Renderer2D::BeginScene();

		s_RenderTargetCamera = camera;
	}
	void Renderer::BeginScene(Camera& camera)
	{
		Transform& transform = camera.m_GameObject.GetComponent<Transform>();
		SetCameraUniforms(Math::Inverse(transform.GetModelMatrix()), camera.GetProjectionMatrix(), transform.GetPosition());
		SetTimeUniforms();
		SetScreenUniform(Vector4(FrameBuffer::GetCurrentFrameBufferSize().x, FrameBuffer::GetCurrentFrameBufferSize().y, 0, 0));
		Renderer2D::BeginScene();
		s_RenderTargetCamera = camera;
	}
	void Renderer::EndScene()
	{
		Renderer2D::EndScene();
	}
	void Renderer::SetRenererAPI(RendererAPI::API api)
	{
		RendererAPI::SetAPI(api);
		RenderCommand::SetRendererAPI(api);
	}

	void Renderer::SetCameraUniforms(Matrix4x4& v, Matrix4x4& p, Vector3& pos)
	{
		s_ShaderUniformData.CameraDataBuffer.GE_MATRIX_V = v;
		s_ShaderUniformData.CameraDataBuffer.GE_MATRIX_P = p;
		s_ShaderUniformData.CameraDataBuffer.GE_MATRIX_VP = p * v;
		s_ShaderUniformData.CameraDataBuffer.GE_CAMERA_POSITION = { pos, 1.0f };

		s_ShaderUniformData.CameraUniformBuffer->SetData(&s_ShaderUniformData.CameraDataBuffer, sizeof(CameraData));
	}
	void Renderer::SetTimeUniforms()
	{
		s_ShaderUniformData.TimeDataBuffer.GE_TIME = Vector4{ Time::GetDeltaTime(), Time::GetRunTime(), Time::GetFixedTime(), Time::GetPhysicsDeltaTime() };

		s_ShaderUniformData.TimeUniformBuffer->SetData(&s_ShaderUniformData.TimeDataBuffer, sizeof(TimeData));
	}
	void Renderer::SetModelUniforms(Transform& transform)
	{
		s_ShaderUniformData.ModelDataBuffer.GE_MATRIX_M = transform.GetModelMatrix();

		s_ShaderUniformData.ModelUniformBuffer->SetData(&s_ShaderUniformData.ModelDataBuffer, sizeof(ModelData));
	}
	void Renderer::SetLightUniforms(Vector3& main_dir, Vector3& main_color)
	{
		s_ShaderUniformData.LigthDataBuffer.GE_MAIN_LIGHT_DIRECTION = { main_dir, 0.0f };
		s_ShaderUniformData.LigthDataBuffer.GE_MAIN_LIGHT_COLOR = { main_color, 1.0f };


		s_ShaderUniformData.LigthUniformBuffer->SetData(&s_ShaderUniformData.LigthDataBuffer, sizeof(LigthData));
	}

	void Renderer::SetScreenUniform(Vector4& size)
	{
		s_ShaderUniformData.ScreenDataBuffer.GE_SCREEN_SIZE = size;

		s_ShaderUniformData.ScreenUniformBuffer->SetData(&s_ShaderUniformData.ScreenDataBuffer, sizeof(ScreenData));
	}

	void Renderer::DrawCubeWireframe(Transform& transform, Vector4 color)
	{
		
	}
}

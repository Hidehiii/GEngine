#include "GEpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "GEngine/Core/Time.h"

namespace GEngine
{
	Camera Renderer::s_RenderTargetCamera = Camera();
	Vector3 Renderer::s_RenderTargetCameraPosition = Vector3();
	uint8_t Renderer::s_FramesInFlight = 2;
	uint8_t Renderer::s_CurrentFrame = 0;
	uint32_t Renderer::s_DynamicUniformBufferSizeScaleFactor = 150;
	std::vector<uint32_t> Renderer::s_DynamicUniformBufferOffsetIndices;
	std::vector<uint32_t> Renderer::s_DynamicUniformBufferOffsets;
	std::vector<uint32_t> Renderer::s_DynamicUniformBufferAlignment;

	int Renderer::s_CameraUniformOffsetIndex = 0;
	int Renderer::s_TimeUniformOffsetIndex = 0;
	int Renderer::s_LightUniformOffsetIndex = 0;
	int Renderer::s_ScreenUniformOffsetIndex = 0;

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

		LigthData			LigthDataBuffer;
		Ref<UniformBuffer>	LigthUniformBuffer;

		ScreenData			ScreenDataBuffer;
		Ref<UniformBuffer>	ScreenUniformBuffer;
	};

	static ShaderUniformData s_ShaderUniformData;

	void Renderer::Init()
	{
		// Uniform Buffers的位置是固定的，所以这里直接创建
		// 0 的位置留给自定义
		AddDynamicUniformBuffer(s_ShaderUniformData.CameraUniformBuffer,	sizeof(CameraData), 1, s_CameraUniformOffsetIndex);
		AddDynamicUniformBuffer(s_ShaderUniformData.TimeUniformBuffer,		sizeof(TimeData),	2, s_TimeUniformOffsetIndex);
		AddDynamicUniformBuffer(s_ShaderUniformData.LigthUniformBuffer,		sizeof(LigthData),	3, s_LightUniformOffsetIndex);
		AddDynamicUniformBuffer(s_ShaderUniformData.ScreenUniformBuffer,	sizeof(ScreenData), 4, s_ScreenUniformOffsetIndex);

		s_DynamicUniformBufferOffsets = std::vector<uint32_t>(s_DynamicUniformBufferAlignment.size(), 0);

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}
	void Renderer::BeginScene(const Editor::EditorCamera& camera)
	{
		SetCameraUniforms(camera.GetViewMatrix(), camera.GetProjectionMatrix(), camera.GetPosition());
		SetTimeUniforms();
		SetScreenUniform(Vector4(FrameBuffer::GetCurrentFrameBufferSize().x, FrameBuffer::GetCurrentFrameBufferSize().y, 0, 0));

		Renderer2D::BeginScene();

		s_RenderTargetCamera = camera;
		s_RenderTargetCameraPosition = camera.GetPosition();
	}
	void Renderer::BeginScene(Camera& camera)
	{
		Transform& transform = camera.m_GameObject.GetComponent<Transform>();
		SetCameraUniforms(Math::Inverse(transform.GetModelMatrix()), camera.GetProjectionMatrix(), transform.GetPosition());
		SetTimeUniforms();
		SetScreenUniform(Vector4(FrameBuffer::GetCurrentFrameBufferSize().x, FrameBuffer::GetCurrentFrameBufferSize().y, 0, 0));
		Renderer2D::BeginScene();

		s_RenderTargetCamera = camera;
		s_RenderTargetCameraPosition = camera.m_GameObject.GetComponent<Transform>().m_Position;
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

		s_ShaderUniformData.CameraUniformBuffer->SetData(&s_ShaderUniformData.CameraDataBuffer, sizeof(CameraData), 
			s_DynamicUniformBufferOffsetIndices.at(s_CameraUniformOffsetIndex) * s_DynamicUniformBufferAlignment.at(s_CameraUniformOffsetIndex));

		UpdateDynamicUniformOffset(s_CameraUniformOffsetIndex);
	}
	void Renderer::SetTimeUniforms()
	{
		s_ShaderUniformData.TimeDataBuffer.GE_TIME = Vector4{ Time::GetDeltaTime(), Time::GetRunTime(), Time::GetFixedTime(), Time::GetPhysicsDeltaTime() };

		s_ShaderUniformData.TimeUniformBuffer->SetData(&s_ShaderUniformData.TimeDataBuffer, sizeof(TimeData), 
			s_DynamicUniformBufferOffsetIndices.at(s_TimeUniformOffsetIndex) * s_DynamicUniformBufferAlignment.at(s_TimeUniformOffsetIndex));

		UpdateDynamicUniformOffset(s_TimeUniformOffsetIndex);
	}
	void Renderer::AddDynamicUniformBuffer(Ref<UniformBuffer>& buffer, uint32_t size, uint32_t binding, int& index)
	{
		index = s_DynamicUniformBufferOffsetIndices.size();
		s_DynamicUniformBufferOffsetIndices.push_back(0);

		uint32_t minUboAligment = RenderCommand::GetMinUniformBufferOffsetAlignment();
		size = (size + minUboAligment - 1) & ~(minUboAligment - 1);
		s_DynamicUniformBufferAlignment.push_back(size);
		GE_CORE_ASSERT(size * s_DynamicUniformBufferSizeScaleFactor <= RenderCommand::GetMaxUniformBufferSize(), "Dynamic UBO could not be larger than the max size of UBO!");
		buffer = UniformBuffer::Create(size * s_DynamicUniformBufferSizeScaleFactor, binding);

		s_DynamicUniformBufferOffsets.push_back(0);
	}
	void Renderer::SetLightUniforms(Vector3& main_dir, Vector3& main_color)
	{
		s_ShaderUniformData.LigthDataBuffer.GE_MAIN_LIGHT_DIRECTION = { main_dir, 0.0f };
		s_ShaderUniformData.LigthDataBuffer.GE_MAIN_LIGHT_COLOR = { main_color, 1.0f };


		s_ShaderUniformData.LigthUniformBuffer->SetData(&s_ShaderUniformData.LigthDataBuffer, sizeof(LigthData), 
			s_DynamicUniformBufferOffsetIndices.at(s_LightUniformOffsetIndex) * s_DynamicUniformBufferAlignment.at(s_LightUniformOffsetIndex));

		UpdateDynamicUniformOffset(s_LightUniformOffsetIndex);
	}

	void Renderer::SetScreenUniform(Vector4& size)
	{
		s_ShaderUniformData.ScreenDataBuffer.GE_SCREEN_SIZE = size;

		s_ShaderUniformData.ScreenUniformBuffer->SetData(&s_ShaderUniformData.ScreenDataBuffer, sizeof(ScreenData), 
			s_DynamicUniformBufferOffsetIndices.at(s_ScreenUniformOffsetIndex) * s_DynamicUniformBufferAlignment.at(s_ScreenUniformOffsetIndex));

		UpdateDynamicUniformOffset(s_ScreenUniformOffsetIndex);
	}

	void Renderer::UpdateDynamicUniformOffset(int index)
	{
		s_DynamicUniformBufferOffsets.at(index) = s_DynamicUniformBufferAlignment.at(index) * s_DynamicUniformBufferOffsetIndices.at(index);

		s_DynamicUniformBufferOffsetIndices.at(index)++;
		s_DynamicUniformBufferOffsetIndices.at(index) %= s_DynamicUniformBufferSizeScaleFactor;
	}

	void Renderer::SetDynamicUniformBufferSizeScaleFactor(uint32_t factor)
	{
		s_DynamicUniformBufferSizeScaleFactor = factor;
	}

	void Renderer::DrawCubeWireframe(Transform& transform, Vector4 color)
	{
		
	}
}

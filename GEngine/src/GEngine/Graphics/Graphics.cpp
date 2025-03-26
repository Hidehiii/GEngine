#include "GEpch.h"
#include "Graphics.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/UniformBuffer.h"
#include "GEngine/Components/Components.h"
#include "Platform/OpenGL/OpenGLGraphicsAPI.h"
#include "Platform/Vulkan/VulkanGraphicsAPI.h"

namespace GEngine
{
	GraphicsAPI*				Graphics::s_GraphicsAPI				= nullptr;
	uint8_t						Graphics::s_FramesInFlight			= 0;
	uint8_t						Graphics::s_Frame					= 0;
	uint32_t					Graphics::s_CommandBufferCount		= 1000;
	uint32_t					Graphics::s_DynamicUniformCount		= 128;
	uint32_t					Graphics::s_ViewportWidth			= 0;
	uint32_t					Graphics::s_ViewportHeight			= 0;

	struct CameraUniformData
	{
		Matrix4x4 GE_MATRIX_V;
		Matrix4x4 GE_MATRIX_P;
		Matrix4x4 GE_MATRIX_VP;
		Vector4 GE_CAMERA_POSITION;
	};

	struct TimeUniformData
	{
		// x: delta time
		// y: run time
		// z: fixed time
		// w: physical delta time
		Vector4 GE_TIME;
	};

	struct ScreenUniformData
	{
		// x: width
		// y: height
		Vector4	GE_SCREEN_SIZE;
	};

	struct UniformData
	{
		CameraUniformData			CameraData;
		Ref<UniformBufferDynamic>	CameraBuffer;

		TimeUniformData				TimeData;
		Ref<UniformBufferDynamic>	TimeBuffer;

		ScreenUniformData			ScreenData;
		Ref<UniformBufferDynamic>	ScreenBuffer;
	};

	static UniformData	s_UniformData;

	void Graphics::Init(const GraphicsSpecification& spec)
    {
		switch (spec.API)
		{
		case GraphicsAPI::API::None:
			GE_CORE_ASSERT(false, "Renderer api is not supported");
			break;
		case GraphicsAPI::API::OpenGL:
			s_GraphicsAPI = new OpenGLGraphicsAPI();
			break;
		case GraphicsAPI::API::Vulkan:
			s_GraphicsAPI = new VulkanGraphicsAPI();
		default:
			break;
		}

		s_Frame					= 0;
		s_FramesInFlight		= spec.FramesInFlight;
		s_CommandBufferCount	= spec.CommandBufferCount;
		s_DynamicUniformCount	= spec.DynamicUniformCount;
		s_ViewportWidth			= spec.ViewportWidth;
		s_ViewportHeight		= spec.ViewportHeight;

		s_UniformData.CameraBuffer	= UniformBufferDynamic::Create(sizeof(CameraUniformData), s_DynamicUniformCount, 1, true);
		s_UniformData.TimeBuffer	= UniformBufferDynamic::Create(sizeof(TimeUniformData), s_DynamicUniformCount, 2, true);

		s_UniformData.ScreenBuffer	= UniformBufferDynamic::Create(sizeof(ScreenUniformData), s_DynamicUniformCount, 4, true);
    }
	void Graphics::FrameMove()
	{
		s_Frame = s_Frame + 1 < s_FramesInFlight ? s_Frame + 1 : 0;
	}
	void Graphics::SetViewport(uint32_t width, uint32_t height)
	{
		s_ViewportWidth = width;
		s_ViewportHeight = height;
	}
	void Graphics::SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second)
	{
		s_GraphicsAPI->SetCommandsBarrier(first, second);
	}
	GraphicsAPI::API Graphics::GetGraphicsAPI()
	{
		return s_GraphicsAPI->GetAPI();
	}
	uint8_t Graphics::GetFramesInFlight()
	{
		return s_FramesInFlight;
	}
	uint8_t Graphics::GetFrame()
	{
		return s_Frame;
	}
	uint32_t Graphics::GetCommandBufferCount()
	{
		return s_CommandBufferCount;
	}
	Ref<CommandBuffer> Graphics::GetGraphicsCommandBuffer()
	{
		return s_GraphicsAPI->GetGraphicsCommandBuffer();
	}
	Ref<CommandBuffer> Graphics::GetComputeCommandBuffer()
	{
		return s_GraphicsAPI->GetComputeCommandBuffer();
	}
	uint32_t Graphics::GetMaxTextureSize()
	{
		return s_GraphicsAPI->GetMaxTextureSize();
	}
	uint32_t Graphics::GetMaxCombinedTextureCount()
	{
		return s_GraphicsAPI->GetMaxCombinedTextureCount();
	}
	uint32_t Graphics::GetMaxPerStageTextureCount()
	{
		return s_GraphicsAPI->GetMaxPerStageTextureCount();
	}
	uint32_t Graphics::GetMaxTextureArrayLayers()
	{
		return s_GraphicsAPI->GetMaxTextureArrayLayers();
	}
	uint32_t Graphics::GetMinUniformBufferOffsetAlignment()
	{
		return s_GraphicsAPI->GetMinUniformBufferOffsetAlignment();
	}
	uint32_t Graphics::GetMaxUniformBufferSize()
	{
		return s_GraphicsAPI->GetMaxUniformBufferSize();
	}
	Vector3 Graphics::GetMaxComputeWorkGroupCount()
	{
		return s_GraphicsAPI->GetMaxComputeWorkGroupCount();
	}
	Vector3 Graphics::GetMaxComputeWorkGroupSize()
	{
		return s_GraphicsAPI->GetMaxComputeWorkGroupSize();
	}
	uint32_t Graphics::GetMaxComputeWorkGroupInvocations()
	{
		return s_GraphicsAPI->GetMaxComputeWorkGroupInvocations();
	}
	uint32_t Graphics::GetViewportWidth()
	{
		return s_ViewportWidth;
	}
	uint32_t Graphics::GetViewportHeight()
	{
		return s_ViewportHeight;
	}
	void Graphics::UpdateCameraUniform(Camera& camera)
	{
		Transform& transform = camera.m_GameObject.GetComponent<Transform>();
		s_UniformData.CameraData.GE_MATRIX_V	= Math::Inverse(transform.GetModelMatrix());
		s_UniformData.CameraData.GE_MATRIX_P	= camera.GetProjectionMatrix();
		s_UniformData.CameraData.GE_MATRIX_VP	= s_UniformData.CameraData.GE_MATRIX_P * s_UniformData.CameraData.GE_MATRIX_V;
		s_UniformData.CameraData.GE_CAMERA_POSITION = { transform.GetPosition(), 1.0f };

		s_UniformData.CameraBuffer->SetData(&s_UniformData.CameraData, sizeof(CameraUniformData));
	}
	void Graphics::UpdateCameraUniform(const Editor::EditorCamera& camera)
	{
		s_UniformData.CameraData.GE_MATRIX_V = camera.GetViewMatrix();
		s_UniformData.CameraData.GE_MATRIX_P = camera.GetProjectionMatrix();
		s_UniformData.CameraData.GE_MATRIX_VP = s_UniformData.CameraData.GE_MATRIX_P * s_UniformData.CameraData.GE_MATRIX_V;
		s_UniformData.CameraData.GE_CAMERA_POSITION = { camera.GetPosition(), 1.0f };

		s_UniformData.CameraBuffer->SetData(&s_UniformData.CameraData, sizeof(CameraUniformData));
	}
	void Graphics::UpdateTimeUniform(Vector4& time)
	{
		s_UniformData.TimeData.GE_TIME = time;

		s_UniformData.TimeBuffer->SetData(&s_UniformData.TimeData, sizeof(TimeUniformData));
	}
	void Graphics::UpdateScreenUniform(Vector4& size)
	{
		s_UniformData.ScreenData.GE_SCREEN_SIZE = size;

		s_UniformData.ScreenBuffer->SetData(&s_UniformData.ScreenData, sizeof(ScreenUniformData));
	}
}
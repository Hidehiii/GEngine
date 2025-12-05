#pragma once
#include "GEngine/Core/Core.h"
#include "GraphicsAPI.h"

namespace GEngine
{
	class CommandBuffer;

	


	class GENGINE_API Graphics
	{
	public:
		static void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second);
		static void SetReverseDepth(bool reverse);

		static Graphics_API			GetGraphicsAPI();
		static uint8_t				GetFrameCount();
		static uint8_t				GetFrame();
		static uint32_t				GetCommandBufferCount();

		static bool					IsReverseDepth();

		static Ref<CommandBuffer>	GetGraphicsCommandBuffer();
		static Ref<CommandBuffer>	GetComputeCommandBuffer();

		static uint32_t GetMaxTextureSize();
		static uint32_t GetMaxCombinedTextureCount();
		static uint32_t GetMaxPerStageTextureCount();
		static uint32_t GetMaxTextureArrayLayers();

		static uint32_t GetMinUniformBufferOffsetAlignment();
		static uint32_t GetMaxUniformBufferSize();

		static Vector3	GetMaxComputeWorkGroupCount();
		static Vector3	GetMaxComputeWorkGroupSize();
		static uint32_t GetMaxComputeWorkGroupInvocations();

		static uint32_t GetViewportWidth();
		static uint32_t GetViewportHeight();

	private:
		static void Setup(const GraphicsSpecification& spec);
		static void Init();
		static void FrameMove();
		static void SetViewport(uint32_t width, uint32_t height);

		friend class Application;
	private:
		static GraphicsAPI* s_GraphicsAPI;
		static uint8_t		s_FrameCount;
		static uint8_t		s_Frame;
		static uint32_t		s_CommandBufferCount;
		static uint32_t		s_ViewportWidth;
		static uint32_t		s_ViewportHeight;
		static bool			s_ReverseDepth;
	};
}



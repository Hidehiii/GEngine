#pragma once

#include "GEngine/Core/Core.h"
#include "RendererAPI.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/CommandBuffer.h"
#include "GEngine/Components/Camera/EditorCamera.h"


namespace GEngine
{
	class GENGINE_API RenderCommand
	{
	public:
		inline void static Init();
		inline static void SetClearColor(const Vector4& color);
		inline static void Clear();
		inline static void SetLineWidth(float width);
		inline static void SetPointSize(float size);
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		inline static float GetTime();
		inline static void SetCull(CullMode mode);
		inline static void SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);
		inline static void EnableDepthWrite(bool enabled);
		inline static void SetDepthTest(CompareOperation op);
		

		inline static void DrawTriangles(CommandBuffer* buffer, const uint32_t indexcount);
		inline static void DrawTriangleInstance(CommandBuffer* buffer, const uint32_t indexcount, const uint32_t instanceCount);
		inline static void DrawLines(CommandBuffer* buffer, const uint32_t indexcount);
		inline static void DrawPoints(CommandBuffer* buffer, const uint32_t indexcount);

		static Ref<CommandBuffer>	GetGraphicsCommandBuffer();
		static Ref<CommandBuffer>	GetComputeCommandBuffer();


		inline static std::vector<std::string> GetExtensions();

		inline static uint32_t GetMaxTextureSize();
		inline static uint32_t GetMaxCombinedTextureCount();
		inline static uint32_t GetMaxPerStageTextureCount();
		inline static uint32_t GetMaxTextureArrayLayers();

		inline static uint32_t GetMinUniformBufferOffsetAlignment();
		inline static uint32_t GetMaxUniformBufferSize();

		// compute
		inline static Vector3 GetMaxComputeWorkGroupCount();
		inline static Vector3 GetMaxComputeWorkGroupSize();
		inline static uint32_t GetMaxComputeWorkGroupInvocations();

		inline static void RegisterSynchronousCommands(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second);

		inline static void Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z);
	private:
		inline static void SetRendererAPI(RendererAPI::API api);
	private:
		inline static RendererAPI* s_RendererAPI;

		friend class Renderer;
	};
}




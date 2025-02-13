#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/RenderBuffer.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include "Material.h"

namespace GEngine
{
	class CommandBuffer;
	class FrameBuffer;

	class GENGINE_API RendererAPI
	{
	public:
		enum class API
		{
			None = 0, 
			OpenGL = 1,
			Vulkan = 2
		};

	public:
		virtual void Init() = 0;
		virtual void Uninit() = 0;

		virtual void SetClearColor(const Vector4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawTriangles(uint32_t indexCount = 0) = 0;
		virtual void DrawLines(uint32_t indexCount = 0) = 0;
		virtual void DrawPoints(uint32_t indexCount = 0) = 0;

		virtual void DrawTrianglesInstance(uint32_t indexCount, uint32_t instanceCount) = 0;

		virtual void SetLineWidth(float width) = 0;
		virtual void SetPointSize(float size) = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void EnableDepthWrite(bool enabled) = 0;
		virtual void SetDepthTest(CompareOperation op) = 0;
		virtual void SetCull(CullMode mode) = 0;
		virtual void SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) = 0;

		virtual float GetTime() = 0;

		virtual void			BeginGraphicsCommand() = 0;
		virtual void			EndGraphicsCommand() = 0;
		virtual CommandBuffer*	BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Camera& camera) = 0;
		virtual CommandBuffer*	BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera) = 0;
		virtual void			EndGraphicsCommand(CommandBuffer* buffer) = 0;
		virtual void			BeginSecondaryCommand() = 0;
		virtual void			EndSecondaryCommand() = 0;

		virtual std::vector<std::string> GetExtensions() = 0;

		virtual uint32_t GetMaxTextureSize() = 0;
		virtual uint32_t GetMaxCombinedTextureCount() = 0;
		virtual uint32_t GetMaxPerStageTextureCount() = 0;
		virtual uint32_t GetMaxTextureArrayLayers() = 0;

		virtual uint32_t GetMinUniformBufferOffsetAlignment() = 0;
		virtual uint32_t GetMaxUniformBufferSize() = 0;

		// compute
		virtual Vector3 GetMaxComputeWorkGroupCount() = 0;
		virtual Vector3 GetMaxComputeWorkGroupSize() = 0;
		virtual uint32_t GetMaxComputeWorkGroupInvocations() = 0;

		virtual void BeginComputeCommand() = 0;
		virtual void EndComputeCommand() = 0;
		virtual void Compute(const uint32_t x, const uint32_t y, const uint32_t z) = 0;


		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }
		inline static API Current() { return s_API; }
	private:
		inline static API s_API		= API::OpenGL;
	};

}




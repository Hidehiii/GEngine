#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RendererAPI.h"


namespace GEngine
{
	class GENGINE_API OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void Uninit() override;

		virtual void SetClearColor(const Vector4& color) override;
		virtual void Clear() override;

		virtual void DrawTriangles(uint32_t indexCount = 0) override;
		virtual void DrawLines(uint32_t indexCount = 0) override;
		virtual void DrawPoints(uint32_t indexCount = 0) override;

		virtual void DrawTrianglesInstance(uint32_t indexCount, uint32_t instanceCount) override;

		virtual void DrawTriangles(CommandBuffer* buffer, uint32_t indexCount = 0) override;
		virtual void DrawLines(CommandBuffer* buffer, uint32_t indexCount = 0) override;
		virtual void DrawPoints(CommandBuffer* buffer, uint32_t indexCount = 0) override;

		virtual void DrawTrianglesInstance(CommandBuffer* buffer, uint32_t indexCount, uint32_t instanceCount) override;

		virtual void EnableDepthWrite(bool enabled) override;
		virtual void SetDepthTest(CompareOperation op) override;
		virtual void SetCull(CullMode mode) override;
		virtual void SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) override;

		virtual void SetLineWidth(float width) override;
		virtual void SetPointSize(float size) override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void			BeginGraphicsCommand() override {}
		virtual void			EndGraphicsCommand() override{}
		virtual Ref<CommandBuffer>	BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Camera& camera) override;
		virtual Ref<CommandBuffer>	BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera) override;
		virtual void			EndGraphicsCommand(Ref<CommandBuffer> buffer) override;
		virtual void			BeginSecondaryCommand() override {}
		virtual void			EndSecondaryCommand() override {}

		virtual float GetTime() override;

		virtual std::vector<std::string> GetExtensions() override;

		virtual uint32_t GetMaxTextureSize() override;
		virtual uint32_t GetMaxCombinedTextureCount() override;
		virtual uint32_t GetMaxPerStageTextureCount() override;
		virtual uint32_t GetMaxTextureArrayLayers() override;

		virtual uint32_t GetMinUniformBufferOffsetAlignment() override;
		virtual uint32_t GetMaxUniformBufferSize() override;

		// compute
		virtual Vector3 GetMaxComputeWorkGroupCount() override;
		virtual Vector3 GetMaxComputeWorkGroupSize() override;
		virtual uint32_t GetMaxComputeWorkGroupInvocations() override;

		virtual Ref<CommandBuffer>	BeginComputeCommand(Ref<FrameBuffer>& buffer, const Camera& camera) override;
		virtual Ref<CommandBuffer>	BeginComputeCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera) override;
		virtual void			EndComputeCommand(Ref<CommandBuffer> buffer) override;

		virtual void BeginComputeCommand() override {}
		virtual void EndComputeCommand() override {}
		virtual void Compute(const uint32_t x, const uint32_t y, const uint32_t z) override;
		virtual void Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z) override;
	};
}


#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsAPI.h"


namespace GEngine
{
	class GENGINE_API OpenGLGraphicsAPI : public GraphicsAPI
	{
	public:
		OpenGLGraphicsAPI();
		virtual ~OpenGLGraphicsAPI();

		virtual Ref<CommandBuffer> GetGraphicsCommandBuffer() override;
		virtual Ref<CommandBuffer> GetComputeCommandBuffer() override;

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

		virtual void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second) override {}


		virtual void Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z) override;
	};
}


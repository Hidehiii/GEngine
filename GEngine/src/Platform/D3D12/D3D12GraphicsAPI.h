#pragma once

#include "GEngine/Graphics/GraphicsAPI.h"

namespace GEngine
{
	class GENGINE_API D3D12GraphicsAPI : public GraphicsAPI
	{
	public:
		D3D12GraphicsAPI();
		virtual ~D3D12GraphicsAPI();

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

		virtual void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second) override;
	};
}



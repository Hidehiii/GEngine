#pragma once

#include "GEngine/Compute/ComputePipeline.h"
#include "Platform/D3D12/D3D12Material.h"
#include <wrl.h>
#include <d3d12.h>

namespace GEngine
{
	class GENGINE_API D3D12ComputePipeline : public ComputePipeline
	{
	public:
		explicit D3D12ComputePipeline(const Ref<Material>& material);
		virtual ~D3D12ComputePipeline() override = default;

		virtual Ref<Material> GetMaterial() override;
		virtual void SetMaterial(Ref<Material>& material) override;

	protected:
		virtual void Compute(CommandBuffer* cmdBuffer, uint32_t pass, uint32_t x, uint32_t y, uint32_t z) override;

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState(uint32_t pass);
		void PrepareCompute(CommandBuffer* cmdBuffer, uint32_t pass);

	private:
		Ref<D3D12Material> m_Material;
		std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PipelineStates;

		friend class D3D12CommandBuffer;
	};
}

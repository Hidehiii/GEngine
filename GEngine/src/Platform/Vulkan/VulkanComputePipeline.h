#pragma once
#include "GEngine/Compute/ComputePipeline.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace GEngine
{
	class GENGINE_API VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(const Ref<Material>& material);
		virtual ~VulkanComputePipeline() override;
		virtual void			Compute(uint32_t x, uint32_t y, uint32_t z) override;
		virtual Ref<Material>	GetMaterial() override;
		virtual void			SetMaterial(Ref<Material>& material) override;
	protected:
		virtual void Compute(CommandBuffer* cmdBuffer, uint32_t x, uint32_t y, uint32_t z) override;

		friend class VulkanCommandBuffer;
	private:
		void CreatePipeline();
		void PrepareCompute();
		void PrepareCompute(CommandBuffer* cmdBuffer);
	private:
		Ref<VulkanMaterial>									m_Material;

		VkPipelineLayout									m_PipelineLayout;
		VkPipeline											m_ComputePipeline = nullptr;
		VkPipelineCache										m_PipelineCache;
		bool												m_RecreatePipeline = false;
	};
}



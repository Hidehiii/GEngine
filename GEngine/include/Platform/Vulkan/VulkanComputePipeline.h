#pragma once
#include "GEngine/Compute/ComputePipeline.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace GEngine
{
	struct VulkanComputePipelineInfo
	{
		VkPipeline	ComputePipeline;
		int			Pass;
	};

	class GENGINE_API VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(const Ref<Material>& material);
		virtual ~VulkanComputePipeline() override;
		virtual Ref<Material>	GetMaterial() override;
		virtual void			SetMaterial(Ref<Material>& material) override;

		bool operator==(const VulkanComputePipeline& other) const
		{
			return m_Material == other.m_Material;
		}
	protected:
		virtual void Compute(CommandBuffer* cmdBuffer, uint32_t pass, uint32_t x, uint32_t y, uint32_t z) override;

		friend class VulkanCommandBuffer;
	private:
		VkPipeline GetPipeline(const uint32_t& pass);
		void PrepareCompute(CommandBuffer* cmdBuffer, const uint32_t& pass);
	private:
		Ref<VulkanMaterial>									m_Material;

		VkPipelineCache										m_PipelineCache;
		std::vector<VulkanComputePipelineInfo>				m_ComputePipelines;
		bool												m_RecreatePipeline = false;
	};
}



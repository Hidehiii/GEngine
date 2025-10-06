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
	protected:
		virtual void Compute(CommandBuffer* cmdBuffer, int pass, uint32_t x, uint32_t y, uint32_t z) override;

		friend class VulkanCommandBuffer;
	private:
		VkPipeline GetPipeline(const int& pass);
		void PrepareCompute(CommandBuffer* cmdBuffer, const int& pass);
	private:
		Ref<VulkanMaterial>									m_Material;

		VkPipelineLayout									m_PipelineLayout;
		VkPipelineCache										m_PipelineCache;
		std::vector<VulkanComputePipelineInfo>				m_ComputePipelines;
		bool												m_RecreatePipeline = false;
	};
}



#pragma once
#include "GEngine/Compute/ComputePipeline.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace GEngine
{
	struct VulkanComputePipelineInfo
	{
		VkPipeline	ComputePipeline;
		std::string Pass;
	};

	class GENGINE_API VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(const Ref<Material>& material);
		virtual ~VulkanComputePipeline() override;
		virtual Ref<Material>	GetMaterial() override;
		virtual void			SetMaterial(Ref<Material>& material) override;
	protected:
		virtual void Compute(CommandBuffer* cmdBuffer, const std::string& pass, uint32_t x, uint32_t y, uint32_t z) override;

		friend class VulkanCommandBuffer;
	private:
		VkPipeline GetPipeline(const std::string& pass);
		void PrepareCompute(CommandBuffer* cmdBuffer, const std::string& pass);
	private:
		Ref<VulkanMaterial>									m_Material;

		VkPipelineLayout									m_PipelineLayout;
		VkPipelineCache										m_PipelineCache;
		std::vector<VulkanComputePipelineInfo>				m_ComputePipelines;
		bool												m_RecreatePipeline = false;
	};
}



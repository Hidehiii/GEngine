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

	private:
		void CreateDescriptorSetAndLayout();
		void UpdateDescriptorSet(int index);
		void UpdateAllDescriptorSet();
		void CreatePipeline();
		void PrepareCompute();
	private:
		Ref<VulkanMaterial>									m_Material;

		VkDescriptorSetLayout								m_DescriptorSetLayout;
		std::vector<VkDescriptorSet>						m_DescriptorSets;

		VkPipelineLayout									m_PipelineLayout;
		VkPipeline											m_ComputePipeline = nullptr;
		VkPipelineCache										m_PipelineCache;
		bool												m_RecreatePipeline = false;
		uint8_t												m_NeedUpdateDescripotrSetFrameCount = 0;
	};
}



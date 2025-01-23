#pragma once
#include "GEngine/Renderer/Pipeline.h"
#include "Platform/Vulkan/VulkanBuffer.h"

#include "Platform/Vulkan/VulkanMaterial.h"



namespace GEngine
{

	class GENGINE_API VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~VulkanPipeline() override;
		virtual void PrepareRender();
		virtual void Render(uint32_t instanceCount = 1, uint32_t indexCount = 0) override;
		virtual Ref<VertexBuffer> GetVertexBuffer() override;
		virtual void SetVertexBuffer(Ref<VertexBuffer>& buffer) override;
		virtual Ref<Material> GetMaterial() override;
		virtual void SetMaterial(Ref<Material>& material) override;
	private:
		// 理论上只要后面不改动ubo和贴图只需要创建和更新一次
		void CreateDescriptorSetAndLayout();
		void UpdateDescriptorSet(int index);
		void UpdateAllDescriptorSet();
		void CreatePipeline();
	private:
		Ref<VulkanMaterial>									m_Material;
		Ref<VulkanVertexBuffer>								m_VertexBuffer;

		VkDescriptorSetLayout								m_DescriptorSetLayout;
		std::vector<VkDescriptorSet>						m_DescriptorSets;

		std::vector<VkDynamicState> m_DynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			
			//VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_CULL_MODE,
			VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
			VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,

			//VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
			//VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
			//VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
			//VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT
		};
		
		VkPipelineLayout									m_PipelineLayout;
		VkPipeline											m_GraphicsPipeline = nullptr;
		bool												m_RecreatePipeline = false;
		uint8_t												m_NeedUpdateDescripotrSetFrameCount = 0;
		VkPipelineCache										m_PipelineCache;
	};
}



#pragma once
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanMaterial.h"



namespace GEngine
{
	// 用于识别当前pass和pipeline是否兼容
	struct VulkanGraphicsPipelineInfo
	{
		VkPipeline		GraphicsPipeline;
		VkRenderPass	RenderPass;
		std::string		Pass;
	};

	class GENGINE_API VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~VulkanGraphicsPipeline() override;
		

		virtual Ref<VertexBuffer>	GetVertexBuffer() override;
		virtual void				SetVertexBuffer(Ref<VertexBuffer>& buffer) override;

		virtual Ref<Material>	GetMaterial() override;
		virtual void			SetMaterial(Ref<Material>& material) override;

	protected:
		virtual void Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, std::string pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

	private:
		void PrepareRender(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, const std::string& pass);
		VkPipeline GetPipeline(const Ref<FrameBuffer>& frameBuffer, const std::string& pass);
	private:
		Ref<VulkanMaterial>									m_Material;
		Ref<VulkanVertexBuffer>								m_VertexBuffer;

		std::vector<VkDynamicState> m_DynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			
			VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_CULL_MODE,
			VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
			VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
			VK_DYNAMIC_STATE_DEPTH_BOUNDS,

			//VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
			//VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
			//VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
			//VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT
		};
		
		VkPipelineLayout									m_PipelineLayout;
		std::vector<VulkanGraphicsPipelineInfo>				m_GraphicsPipelines;
		VkPipelineCache										m_PipelineCache;
		bool												m_RecreatePipeline = false;

		friend class VulkanCommandBuffer;
	};
}



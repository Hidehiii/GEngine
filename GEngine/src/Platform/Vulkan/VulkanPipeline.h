#pragma once
#include "GEngine/Renderer/Pipeline.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanVertexArray.h"
#include "Platform/Vulkan/VulkanMaterial.h"



namespace GEngine
{

	class GENGINE_API VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const Ref<Material>& material, const Ref<VertexArray>& vertexArray, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~VulkanPipeline() override;
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual Ref<VertexArray> GetVertexArray() override { return std::static_pointer_cast<VertexArray>(m_VertexArray); }
		virtual Ref<VertexBuffer> GetVertexBuffer() override { return std::static_pointer_cast<VertexBuffer>(m_VertexBuffer); }
		virtual Ref<Material> GetMaterial() override { return std::static_pointer_cast<Material>(m_Material);}
	private:
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);
		void CreatePipeline();
	private:
		Ref<VulkanMaterial>			m_Material;
		Ref<VulkanVertexArray>		m_VertexArray;
		Ref<VulkanVertexBuffer>		m_VertexBuffer;

		VkShaderModule				m_VertexShaderModule;
		VkShaderModule				m_FragmentShaderModule;
		std::vector<VkDynamicState> m_DynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			/*VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_CULL_MODE,
			VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
			VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,

			VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
			VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,*/
		};
		std::vector<VkPipelineShaderStageCreateInfo>		m_ShaderStages;
		VkPipelineVertexInputStateCreateInfo				m_VertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo				m_InputAssembly{};
		VkViewport											m_Viewport{};
		VkRect2D											m_Scissor{};
		VkPipelineViewportStateCreateInfo					m_ViewportState{};
		VkPipelineRasterizationStateCreateInfo				m_Rasterizer{};
		VkPipelineMultisampleStateCreateInfo				m_Multisampling{};
		VkPipelineColorBlendAttachmentState					m_ColorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo					m_ColorBlending{};
		VkPipelineLayout									m_PipelineLayout;
		VkPipeline											m_GraphicsPipeline;

	};
}



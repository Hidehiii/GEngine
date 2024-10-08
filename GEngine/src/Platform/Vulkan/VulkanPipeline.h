#pragma once
#include "GEngine/Core/Core.h"
#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanPipeline
	{
	public:
		VulkanPipeline(const std::vector<uint32_t>& vertex, const std::vector<uint32_t>& fragment);
		~VulkanPipeline();
		static Ref<VulkanPipeline> Create(const std::vector<uint32_t>& vertex, const std::vector<uint32_t>& fragment);

	private:
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code);

	private:
		VkShaderModule				m_VertexShaderModule;
		VkShaderModule				m_FragmentShaderModule;
		std::vector<VkDynamicState> m_DynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_CULL_MODE,
			VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
			VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,

			VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
			VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
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



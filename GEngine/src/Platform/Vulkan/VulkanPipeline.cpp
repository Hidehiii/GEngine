#include "GEpch.h"
#include "VulkanPipeline.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"


namespace GEngine
{
	namespace Utils
	{
		// front和back要反过来，适应viewport的反转
		VkCullModeFlagBits MaterialCullModeToVkCullMode(MaterialCullMode mode)
		{
			switch (mode)
			{
			case MaterialCullMode::None:	return VK_CULL_MODE_NONE;
			case MaterialCullMode::Front:	return VK_CULL_MODE_BACK_BIT;
			case MaterialCullMode::Back:	return VK_CULL_MODE_FRONT_BIT; 
			default:
				break;
			}
		}
	}

    VulkanPipeline::VulkanPipeline(const Ref<Material>& material, const Ref<VertexArray>& vertexArray, const Ref<VertexBuffer>& vertexBuffer)
    {
        m_Material              = std::dynamic_pointer_cast<VulkanMaterial>(material);
        m_VertexArray           = std::dynamic_pointer_cast<VulkanVertexArray>(vertexArray);
        m_VertexBuffer          = std::dynamic_pointer_cast<VulkanVertexBuffer>(vertexBuffer);

		
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
    }

    void VulkanPipeline::Bind()
    {
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), "There is no commandbuffer be using");
		GE_CORE_ASSERT(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer(), "There is no framebuffer be using");

		m_Material->UploadData();

		if (m_FirstCreatePipeline)
		{
			CreateDescriptorSetAndLayout();
			UpdateDescriptorSet();
			CreatePipeline();
			m_FirstCreatePipeline = false;
		}
		if (m_RecreatePipeline)
		{
			vkQueueWaitIdle(VulkanContext::Get()->GetGraphicsQueue());
			vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_GraphicsPipeline, nullptr);
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), 1, &m_DescriptorSet);
			CreateDescriptorSetAndLayout();
			UpdateDescriptorSet();
			CreatePipeline();
			m_RecreatePipeline = false;
		}
		
        vkCmdBindPipeline(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		m_Viewport.x		= 0.0f;
		m_Viewport.y		= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight();
		m_Viewport.width	= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetWidth();
		m_Viewport.height	= -(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight());
		m_Viewport.minDepth = 0.0f;
		m_Viewport.maxDepth = 1.0f;
		vkCmdSetViewport(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0, 1, &m_Viewport);

		m_Scissor.offset = { 0, 0 };
		m_Scissor.extent = { (unsigned int)(int)VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetWidth(), (unsigned int)(int)VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight() };
		vkCmdSetScissor(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0, 1, &m_Scissor);

		

		vkCmdBindDescriptorSets(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
    }

    void VulkanPipeline::Unbind()
    {

    }


    VkShaderModule VulkanPipeline::CreateShaderModule(const std::vector<uint32_t>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode    = code.data();
        VkShaderModule      shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::Get()->GetDevice(), &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }
	void VulkanPipeline::CreateDescriptorSetAndLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;
		std::vector<VulkanUniformBuffer*> publicUniformBuffer		= VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			layoutBindings.push_back(buffer->GetDescriptorSetLayoutBinding());
		}
		layoutBindings.push_back(m_Material->GetUniformBuffer()->GetDescriptorSetLayoutBinding());
		// TODO : 后面贴图需要放在材质里而不是外部绑定
		auto texture2Ds		= m_Material->GetGetTexture2Ds();
		for (auto& texture2D : texture2Ds)
		{
			layoutBindings.push_back(std::dynamic_pointer_cast<VulkanTexture2D>(texture2D.Texture)->GetDescriptorSetLayoutBinding());
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount			= static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings			= layoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::Get()->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout));

		// 只有一个set，需要兼容gl没有set的概念
		VkDescriptorSetAllocateInfo		allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool		= VulkanContext::Get()->GetDescriptorPool();
		allocInfo.descriptorSetCount	= 1;
		allocInfo.pSetLayouts			= &m_DescriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::Get()->GetDevice(), &allocInfo, &m_DescriptorSet));
	}
	void VulkanPipeline::UpdateDescriptorSet()
	{
		// 更新公共uniform buffer
		std::vector<VulkanUniformBuffer*> publicUniformBuffer = VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			VkDescriptorBufferInfo			bufferInfo = buffer->GetDescriptorBufferInfo();

			VkWriteDescriptorSet			descriptorWrite{};
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSet;
			descriptorWrite.dstBinding		= buffer->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= &bufferInfo;
			descriptorWrite.pImageInfo		= nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
		// 更新材质的uniform buffer
		{
			VkDescriptorBufferInfo			bufferInfo = m_Material->GetUniformBuffer()->GetDescriptorBufferInfo();

			VkWriteDescriptorSet			descriptorWrite{};
			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSet;
			descriptorWrite.dstBinding		= m_Material->GetUniformBuffer()->GetBinding();
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo		= &bufferInfo;
			descriptorWrite.pImageInfo		= nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
		// TODO: 后面贴图也要放进材质，由这里统一获取进行绑定更新
		{
			auto texture2Ds					= m_Material->GetGetTexture2Ds();
			for (auto& texture2D : texture2Ds)
			{
				VkDescriptorImageInfo			imageInfo = std::dynamic_pointer_cast<VulkanTexture2D>(texture2D.Texture)->GetDescriptorImageInfo();
				VkWriteDescriptorSet			descriptorWrite{};
				descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet			= m_DescriptorSet;
				descriptorWrite.dstBinding		= std::dynamic_pointer_cast<VulkanTexture2D>(texture2D.Texture)->GetBinding();
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo		= nullptr;
				descriptorWrite.pImageInfo		= &imageInfo; 
				descriptorWrite.pTexelBufferView = nullptr; // Optional

				vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}
    void VulkanPipeline::CreatePipeline()
    {
		m_VertexShaderModule		= CreateShaderModule(m_Material->GetShader()->GetVertexShaderSource());
		m_FragmentShaderModule		= CreateShaderModule(m_Material->GetShader()->GetFragmentShaderSource());

		VkPipelineShaderStageCreateInfo		vertexShaderStageInfo{};
		vertexShaderStageInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageInfo.stage			= VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderStageInfo.module		= m_VertexShaderModule;
		vertexShaderStageInfo.pName			= "main";

		VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderStageInfo.stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderStageInfo.module		= m_FragmentShaderModule;
		fragmentShaderStageInfo.pName		= "main";

		m_ShaderStages						= { vertexShaderStageInfo, fragmentShaderStageInfo };

		VkPipelineDynamicStateCreateInfo			dynamicStateCreateInfo{};
		dynamicStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount	= m_DynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates		= m_DynamicStates.data();

		m_VertexInputInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		m_VertexInputInfo.vertexBindingDescriptionCount		= static_cast<uint32_t>(m_VertexArray->GetVertexInputBindingDescriptions().size());
		auto												bindingDescriptions = m_VertexArray->GetVertexInputBindingDescriptions();
		m_VertexInputInfo.pVertexBindingDescriptions		= bindingDescriptions.data();
		m_VertexInputInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(m_VertexArray->GetVertexInputAttributeDescriptions().size());
		auto												attributeDescription = m_VertexArray->GetVertexInputAttributeDescriptions();
		m_VertexInputInfo.pVertexAttributeDescriptions		= attributeDescription.data();

		m_InputAssembly.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		m_InputAssembly.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		m_InputAssembly.primitiveRestartEnable	= VK_FALSE;

		m_Viewport.x			= 0.0f;
		m_Viewport.y			= 0.0f;
		m_Viewport.width		= (float)VulkanContext::Get()->GetSwapChainExtent().width;
		m_Viewport.height		= (float)VulkanContext::Get()->GetSwapChainExtent().height;
		m_Viewport.minDepth		= 0.0f;
		m_Viewport.maxDepth		= 1.0f;

		m_Scissor.offset = { 0, 0 };
		m_Scissor.extent = VulkanContext::Get()->GetSwapChainExtent();

		m_ViewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		m_ViewportState.viewportCount	= 1;
		m_ViewportState.pViewports		= &m_Viewport;
		m_ViewportState.scissorCount	= 1;
		m_ViewportState.pScissors		= &m_Scissor;

		m_Rasterizer.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		m_Rasterizer.depthClampEnable			= VK_FALSE;
		m_Rasterizer.rasterizerDiscardEnable	= VK_FALSE;
		m_Rasterizer.polygonMode				= VK_POLYGON_MODE_FILL;
		m_Rasterizer.lineWidth					= 1.0f;
		m_Rasterizer.cullMode					= Utils::MaterialCullModeToVkCullMode(m_Material->GetCullMode());
		m_Rasterizer.frontFace					= VK_FRONT_FACE_CLOCKWISE;

		m_Multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		m_Multisampling.sampleShadingEnable		= VK_FALSE;
		m_Multisampling.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
		m_Multisampling.minSampleShading		= 1.0f;
		m_Multisampling.pSampleMask				= nullptr;
		m_Multisampling.alphaToCoverageEnable	= VK_FALSE;
		m_Multisampling.alphaToOneEnable		= VK_FALSE;

		m_ColorBlendAttachment.colorWriteMask		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_ColorBlendAttachment.blendEnable			= VK_FALSE;
		m_ColorBlendAttachment.srcColorBlendFactor	= (VkBlendFactor)m_Material->GetBlendSourceFactor();
		m_ColorBlendAttachment.dstColorBlendFactor	= (VkBlendFactor)m_Material->GetBlendDestinationFactor();
		m_ColorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD;
		m_ColorBlendAttachment.srcAlphaBlendFactor	= (VkBlendFactor)m_Material->GetBlendSourceFactor();
		m_ColorBlendAttachment.dstAlphaBlendFactor	= (VkBlendFactor)m_Material->GetBlendDestinationFactor();
		m_ColorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD;

		m_ColorBlending.sType						= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		m_ColorBlending.logicOpEnable				= VK_FALSE;
		m_ColorBlending.logicOp						= VK_LOGIC_OP_COPY;
		m_ColorBlending.attachmentCount				= 1;
		m_ColorBlending.pAttachments				= &m_ColorBlendAttachment;
		m_ColorBlending.blendConstants[0]			= 0.0f;
		m_ColorBlending.blendConstants[1]			= 0.0f;
		m_ColorBlending.blendConstants[2]			= 0.0f;
		m_ColorBlending.blendConstants[3]			= 0.0f;

		VkPipelineLayoutCreateInfo					pipelineLayoutInfo{};
		pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			= 1;
		pipelineLayoutInfo.pSetLayouts				= &m_DescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount	= 0;
		pipelineLayoutInfo.pPushConstantRanges		= nullptr;

		VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::Get()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		VkPipelineDepthStencilStateCreateInfo	depthStencil{};
		depthStencil.sType						= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable			= m_Material->GetEnableDepthTest() ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable			= m_Material->GetEnableDepthWrite() ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp				= VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable		= VK_FALSE;
		depthStencil.minDepthBounds				= 0.0f; // Optional
		depthStencil.maxDepthBounds				= 1.0f; // Optional
		depthStencil.stencilTestEnable			= VK_FALSE;
		depthStencil.front						= {}; // Optional
		depthStencil.back						= {}; // Optional

		VkGraphicsPipelineCreateInfo        pipelineInfo{};
		pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount				= static_cast<uint32_t>(m_ShaderStages.size());
		pipelineInfo.pStages				= m_ShaderStages.data();
		pipelineInfo.pVertexInputState		= &m_VertexInputInfo;
		pipelineInfo.pInputAssemblyState	= &m_InputAssembly;
		pipelineInfo.pViewportState			= &m_ViewportState;
		pipelineInfo.pRasterizationState	= &m_Rasterizer;
		pipelineInfo.pMultisampleState		= &m_Multisampling;
		pipelineInfo.pDepthStencilState		= &depthStencil;
		pipelineInfo.pColorBlendState		= &m_ColorBlending;
		pipelineInfo.pDynamicState			= &dynamicStateCreateInfo;
		pipelineInfo.layout					= m_PipelineLayout;
		pipelineInfo.renderPass				= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetRenderPass();
		pipelineInfo.subpass				= 0;
		pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex		= -1; // Optional

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanContext::Get()->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

		vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_VertexShaderModule, nullptr);
		vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_FragmentShaderModule, nullptr);
    }
}
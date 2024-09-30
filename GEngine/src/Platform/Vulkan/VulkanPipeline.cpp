#include "GEpch.h"
#include "VulkanPipeline.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "GEngine/Application.h"

namespace GEngine
{
    Ref<VulkanPipeline> VulkanPipeline::Create(const std::vector<uint32_t>& vertex, const std::vector<uint32_t>& fragment)
    {
        Ref<VulkanPipeline> pipeline = CreateRef<VulkanPipeline>(vertex, fragment);
        return pipeline;
    }

    

    VulkanPipeline::VulkanPipeline(const std::vector<uint32_t>& vertex, const std::vector<uint32_t>& fragment)
    {
        m_VertexShaderModule    = CreateShaderModule(vertex);
        m_FragmentShaderModule  = CreateShaderModule(fragment);

        VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
        vertexShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageInfo.module = m_VertexShaderModule;
        vertexShaderStageInfo.pName  = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
        fragmentShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageInfo.module = m_FragmentShaderModule;
        fragmentShaderStageInfo.pName  = "main";

        m_ShaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = m_DynamicStates.size();
        dynamicStateCreateInfo.pDynamicStates    = m_DynamicStates.data();

        m_VertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        m_VertexInputInfo.vertexBindingDescriptionCount   = 0;
        m_VertexInputInfo.pVertexBindingDescriptions      = nullptr;
        m_VertexInputInfo.vertexAttributeDescriptionCount = 0;
        m_VertexInputInfo.pVertexAttributeDescriptions    = nullptr;

        m_InputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        m_InputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        m_InputAssembly.primitiveRestartEnable = VK_FALSE;

        m_Viewport.x          = 0.0f;
        m_Viewport.y          = 0.0f;
        m_Viewport.width      = (float)((VulkanContext*)Application::Get().GetWindow().GetContext())->GetSwapChainExtent().width;
        m_Viewport.height     = (float)((VulkanContext*)Application::Get().GetWindow().GetContext())->GetSwapChainExtent().height;
        m_Viewport.minDepth   = 0.0f;
        m_Viewport.maxDepth   = 1.0f;

        m_Scissor.offset      = {0, 0};
        m_Scissor.extent      = ((VulkanContext*)Application::Get().GetWindow().GetContext())->GetSwapChainExtent();

        m_ViewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        m_ViewportState.viewportCount = 1;
        m_ViewportState.pViewports    = &m_Viewport;
        m_ViewportState.scissorCount  = 1;
        m_ViewportState.pScissors     = &m_Scissor;

        m_Rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_Rasterizer.depthClampEnable        = VK_FALSE;
        m_Rasterizer.rasterizerDiscardEnable = VK_FALSE;
        m_Rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        m_Rasterizer.lineWidth               = 1.0f;
        m_Rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
        m_Rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;

        m_Multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_Multisampling.sampleShadingEnable   = VK_FALSE;
        m_Multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        m_Multisampling.minSampleShading      = 1.0f;
        m_Multisampling.pSampleMask           = nullptr;
        m_Multisampling.alphaToCoverageEnable = VK_FALSE;
        m_Multisampling.alphaToOneEnable      = VK_FALSE;

        m_ColorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        m_ColorBlendAttachment.blendEnable         = VK_FALSE;
        m_ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        m_ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_ColorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
        m_ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        m_ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_ColorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

        m_ColorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_ColorBlending.logicOpEnable     = VK_FALSE;
        m_ColorBlending.logicOp           = VK_LOGIC_OP_COPY;
        m_ColorBlending.attachmentCount   = 1;
        m_ColorBlending.pAttachments      = &m_ColorBlendAttachment;
        m_ColorBlending.blendConstants[0] = 0.0f;
        m_ColorBlending.blendConstants[1] = 0.0f;
        m_ColorBlending.blendConstants[2] = 0.0f;
        m_ColorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pSetLayouts            = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges    = nullptr;

        if (vkCreatePipelineLayout(((VulkanContext*)(Application::Get().GetWindow().GetContext()))->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            GE_CORE_ERROR("Failed to create pipeline layout!");
        }


        vkDestroyShaderModule(((VulkanContext*)(Application::Get().GetWindow().GetContext()))->GetDevice(), m_VertexShaderModule, nullptr);
        vkDestroyShaderModule(((VulkanContext*)(Application::Get().GetWindow().GetContext()))->GetDevice(), m_FragmentShaderModule, nullptr);
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipelineLayout(((VulkanContext*)(Application::Get().GetWindow().GetContext()))->GetDevice(), m_PipelineLayout, nullptr);
    }


    VkShaderModule VulkanPipeline::CreateShaderModule(const std::vector<uint32_t>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode    = code.data();
        VkShaderModule      shaderModule;
        if (vkCreateShaderModule(((VulkanContext*)(Application::Get().GetWindow().GetContext()))->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
			GE_CORE_ERROR("Failed to create shader module!");
			return VkShaderModule();
		}
        return shaderModule;
    }
}
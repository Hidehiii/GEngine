#include "GEpch.h"
#include "VulkanGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
namespace GEngine
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
    {
        m_Material              = std::dynamic_pointer_cast<VulkanMaterial>(material);
        m_VertexBuffer          = std::dynamic_pointer_cast<VulkanVertexBuffer>(vertexBuffer);

		VkPipelineCacheCreateInfo			pipelineCacheInfo{};
		pipelineCacheInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheInfo.initialDataSize	= 0;
		pipelineCacheInfo.pInitialData		= nullptr;

		VK_CHECK_RESULT(vkCreatePipelineCache(VulkanContext::Get()->GetDevice(), &pipelineCacheInfo, nullptr, &m_PipelineCache));
    }

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
		if (VulkanContext::Get()->GetDevice())
		{
			vkDestroyPipelineCache(VulkanContext::Get()->GetDevice(), m_PipelineCache, nullptr);
			for (auto pipeline : m_GraphicsPipelines)
			{
				vkDestroyPipeline(VulkanContext::Get()->GetDevice(), pipeline.GraphicsPipeline, nullptr);
			}
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
		}
        
    }

	void VulkanGraphicsPipeline::PrepareRender(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, const std::string& pass)
	{
		m_Material->Update(cmdBuffer, pass);

		auto cmd = static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer();
		auto vkFrameBuffer = std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer);

		if (m_RecreatePipeline)
		{
			for (auto pipeline : m_GraphicsPipelines)
			{
				vkDestroyPipeline(VulkanContext::Get()->GetDevice(), pipeline.GraphicsPipeline, nullptr);
			}
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
			m_RecreatePipeline = false;
			m_GraphicsPipelines.clear();
		}
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipeline(frameBuffer, pass));

		VkViewport			Viewport{};
		Viewport.x			= 0.0f;
		Viewport.y			= vkFrameBuffer->GetHeight();
		Viewport.width		= vkFrameBuffer->GetWidth();
		Viewport.height		= -(vkFrameBuffer->GetHeight());
		Viewport.minDepth	= 0.0f;
		Viewport.maxDepth	= 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &Viewport);

		VkRect2D		Scissor{};
		Scissor.offset	= { 0, 0 };
		Scissor.extent	= { (unsigned int)vkFrameBuffer->GetWidth(), (unsigned int)vkFrameBuffer->GetHeight() };
		vkCmdSetScissor(cmd, 0, 1, &Scissor);

		vkCmdSetLineWidth(cmd, 1.0f);
		vkCmdSetDepthCompareOp(cmd, Utils::CompareOPToVkCompareOP(m_Material->GetDepthTestOp(pass)));
		vkCmdSetDepthWriteEnable(cmd, m_Material->IsEnableDepthWrite(pass));
		vkCmdSetCullMode(cmd, Utils::CullModeToVkCullMode(m_Material->GetCull(pass)));
		vkCmdSetDepthBounds(cmd, Graphics::IsReverseDepth() ? 1.0f : 0.0f, Graphics::IsReverseDepth() ? 0.0f : 1.0f);
		auto offsets = UniformBufferDynamic::GetGlobalUniformOffsets();
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, m_Material->GetDescriptorSet(Graphics::GetFrame()), offsets.size(), offsets.data());
	}

	void VulkanGraphicsPipeline::Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, std::string pass, uint32_t instanceCount, uint32_t indexCount)
	{
		PrepareRender(cmdBuffer, frameBuffer, pass);
		m_VertexBuffer->Bind(cmdBuffer);
		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexBuffer()->GetCount();
		if (m_VertexBuffer->IsInstanceRendering())
		{
			vkCmdDrawIndexed(((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
		}
		else
		{
			vkCmdDrawIndexed(((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
		}
	}

	Ref<VertexBuffer> VulkanGraphicsPipeline::GetVertexBuffer()
	{
		return (m_VertexBuffer);
	}

	void VulkanGraphicsPipeline::SetVertexBuffer(Ref<VertexBuffer>& buffer)
	{
		m_VertexBuffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(buffer);
		m_RecreatePipeline = true;
	}

	Ref<Material> VulkanGraphicsPipeline::GetMaterial()
	{
		return (m_Material);
	}

	void VulkanGraphicsPipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<VulkanMaterial>(material);
		m_RecreatePipeline = true;
	}
    
	VkPipeline VulkanGraphicsPipeline::GetPipeline(const Ref<FrameBuffer>& frameBuffer, const std::string& pass)
	{
		for (int i = m_GraphicsPipelines.size() - 1; i >= 0; i--)
		{
			if (m_GraphicsPipelines.at(i).RenderPass == std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetVulkanRenderPass() &&
				m_GraphicsPipelines.at(i).Pass == pass)
			{
				return m_GraphicsPipelines.at(i).GraphicsPipeline;
			}
		}
		auto shader = std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader());

		std::string shaderMainFuncName						= m_Material->GetShader()->GetShaderMainFuncName().c_str();
		std::vector<VkPipelineShaderStageCreateInfo>		ShaderStages;

		if (shader->GetShaderModule(ShaderStage::Vertex, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_VERTEX_BIT,
				shader->GetShaderModule(ShaderStage::Vertex, pass),
				shaderMainFuncName.c_str()));
		}
		if (shader->GetShaderModule(ShaderStage::Fragment, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT,
				shader->GetShaderModule(ShaderStage::Fragment, pass),
				shaderMainFuncName.c_str()));
		}
		if (shader->GetShaderModule(ShaderStage::TessellationControl, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				shader->GetShaderModule(ShaderStage::TessellationControl, pass),
				shaderMainFuncName.c_str()));
		}
		if (shader->GetShaderModule(ShaderStage::TessellationEvaluation, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
				shader->GetShaderModule(ShaderStage::TessellationEvaluation, pass),
				shaderMainFuncName.c_str()));
		}
		if (shader->GetShaderModule(ShaderStage::Geometry, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_GEOMETRY_BIT,
				shader->GetShaderModule(ShaderStage::Geometry, pass),
				shaderMainFuncName.c_str()));
		}
		if (shader->GetShaderModule(ShaderStage::Task, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_TASK_BIT_EXT,
				shader->GetShaderModule(ShaderStage::Task, pass),
				shaderMainFuncName.c_str()));
		}
		if (shader->GetShaderModule(ShaderStage::Mesh, pass))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_MESH_BIT_EXT,
				shader->GetShaderModule(ShaderStage::Mesh, pass),
				shaderMainFuncName.c_str()));
		}

		GE_CORE_ASSERT(ShaderStages.size() > 0, " there are no shader stage in graphics pipeline pass{}", pass);
		

		VkPipelineDynamicStateCreateInfo			dynamicStateCreateInfo{};
		dynamicStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount	= m_DynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates		= m_DynamicStates.data();

		VkPipelineVertexInputStateCreateInfo				VertexInputInfo{};
		VertexInputInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputInfo.vertexBindingDescriptionCount		= static_cast<uint32_t>(m_VertexBuffer->GetVertexInputBindingDescription().size());
		auto												bindingDescription = m_VertexBuffer->GetVertexInputBindingDescription();
		VertexInputInfo.pVertexBindingDescriptions			= bindingDescription.data();
		VertexInputInfo.vertexAttributeDescriptionCount		= static_cast<uint32_t>(m_VertexBuffer->GetVertexInputAttributeDescriptions().size());
		auto												attributeDescription = m_VertexBuffer->GetVertexInputAttributeDescriptions();
		VertexInputInfo.pVertexAttributeDescriptions		= attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo				InputAssembly{};
		InputAssembly.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssembly.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		switch (m_VertexBuffer->GetVertexTopologyType())
		{
		case VertexTopology::Point:		InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
		case VertexTopology::Line:		InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
		case VertexTopology::Triangle:	InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
		default: GE_CORE_ASSERT(false, "unkown topolopgy");
		}
		InputAssembly.primitiveRestartEnable	= VK_FALSE;

		VkViewport			Viewport{};
		Viewport.x			= 0.0f;
		Viewport.y			= 0.0f;
		Viewport.width		= (float)VulkanContext::Get()->GetSwapChainExtent().width;
		Viewport.height		= (float)VulkanContext::Get()->GetSwapChainExtent().height;
		Viewport.minDepth	= 0.0f;
		Viewport.maxDepth	= 1.0f;

		VkRect2D											Scissor{};
		Scissor.offset = { 0, 0 };
		Scissor.extent = VulkanContext::Get()->GetSwapChainExtent();

		VkPipelineViewportStateCreateInfo					ViewportState{};
		ViewportState.sType									= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportState.viewportCount							= 1;
		ViewportState.pViewports							= &Viewport;
		ViewportState.scissorCount							= 1;
		ViewportState.pScissors								= &Scissor;

		VkPipelineRasterizationStateCreateInfo				Rasterizer{};
		Rasterizer.sType									= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		Rasterizer.depthClampEnable							= VK_FALSE;
		Rasterizer.rasterizerDiscardEnable					= VK_FALSE;
		Rasterizer.polygonMode								= VK_POLYGON_MODE_FILL;
		Rasterizer.lineWidth								= 1.0f;
		Rasterizer.cullMode									= Utils::CullModeToVkCullMode(m_Material->GetCull());
		Rasterizer.frontFace								= VK_FRONT_FACE_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo				Multisampling{};
		Multisampling.sType									= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		Multisampling.sampleShadingEnable					= VK_FALSE;
		Multisampling.rasterizationSamples					= Utils::SampleCountToVulkanFlag(frameBuffer->GetSpecification().Samples);
		Multisampling.minSampleShading						= 1.0f;
		Multisampling.pSampleMask							= nullptr;
		Multisampling.alphaToCoverageEnable					= VK_FALSE;
		Multisampling.alphaToOneEnable						= VK_FALSE;

		VkPipelineColorBlendAttachmentState					ColorBlendAttachment{};
		ColorBlendAttachment.colorWriteMask					= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachment.blendEnable					= VK_TRUE;
		ColorBlendAttachment.srcColorBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendColorSrc());
		ColorBlendAttachment.dstColorBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendColorDst());
		ColorBlendAttachment.colorBlendOp					= VK_BLEND_OP_ADD;
		switch (m_Material->GetBlendColor())
		{
		case BlendMode::None:
			ColorBlendAttachment.blendEnable = VK_FALSE;
			break;
		case BlendMode::Add:
			ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			break;
		case BlendMode::Substract:
			ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_SUBTRACT;
			break;
		case BlendMode::ReverseSubstract:
			ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
			break;
		case BlendMode::Min:
			ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_MIN;
			break;
		case BlendMode::Max:
			ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_MAX;
			break;
		default:
			break;
		}
		ColorBlendAttachment.srcAlphaBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendAlphaSrc());
		ColorBlendAttachment.dstAlphaBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendAlphaDst());
		ColorBlendAttachment.alphaBlendOp					= VK_BLEND_OP_ADD;
		switch (m_Material->GetBlendAlpha())
		{
		case BlendMode::None:
			ColorBlendAttachment.blendEnable = VK_FALSE;
			break;
		case BlendMode::Add:
			ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			break;
		case BlendMode::Substract:
			ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
			break;
		case BlendMode::ReverseSubstract:
			ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
			break;
		case BlendMode::Min:
			ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_MIN;
			break;
		case BlendMode::Max:
			ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_MAX;
			break;
		default:
			break;
		}
		std::vector<VkPipelineColorBlendAttachmentState>	attachmentsBlend(frameBuffer->GetColorRTCount(), ColorBlendAttachment);

		VkPipelineColorBlendStateCreateInfo					ColorBlending{};
		ColorBlending.sType									= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlending.logicOpEnable							= VK_FALSE;
		ColorBlending.logicOp								= VK_LOGIC_OP_COPY;
		ColorBlending.attachmentCount						= static_cast<uint32_t>(attachmentsBlend.size());
		ColorBlending.pAttachments							= attachmentsBlend.data();
		ColorBlending.blendConstants[0]						= 0.0f;
		ColorBlending.blendConstants[1]						= 0.0f;
		ColorBlending.blendConstants[2]						= 0.0f;
		ColorBlending.blendConstants[3]						= 0.0f;

		VkPipelineLayoutCreateInfo					pipelineLayoutInfo{};
		pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			= 1;
		pipelineLayoutInfo.pSetLayouts				= m_Material->GetDescriptorSetLayout();
		pipelineLayoutInfo.pushConstantRangeCount	= 0;
		pipelineLayoutInfo.pPushConstantRanges		= nullptr;

		VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::Get()->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		VkPipelineDepthStencilStateCreateInfo	depthStencil{};
		depthStencil.sType						= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable			= VK_TRUE;
		depthStencil.depthWriteEnable			= m_Material->IsEnableDepthWrite() ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp				= Utils::CompareOPToVkCompareOP(m_Material->GetDepthTestOp());
		depthStencil.depthBoundsTestEnable		= VK_FALSE;
		depthStencil.minDepthBounds				= Graphics::IsReverseDepth() ? 1.0f : 0.0f;
		depthStencil.maxDepthBounds				= Graphics::IsReverseDepth() ? 0.0f : 1.0f;
		depthStencil.stencilTestEnable			= VK_FALSE;
		depthStencil.front						= {}; // Optional
		depthStencil.back						= {}; // Optional

		VkGraphicsPipelineCreateInfo        pipelineInfo{};
		pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount				= static_cast<uint32_t>(ShaderStages.size());
		pipelineInfo.pStages				= ShaderStages.data();
		pipelineInfo.pVertexInputState		= &VertexInputInfo;
		pipelineInfo.pInputAssemblyState	= &InputAssembly;
		pipelineInfo.pViewportState			= &ViewportState;
		pipelineInfo.pRasterizationState	= &Rasterizer;
		pipelineInfo.pMultisampleState		= &Multisampling;
		pipelineInfo.pDepthStencilState		= &depthStencil;
		pipelineInfo.pColorBlendState		= &ColorBlending;
		pipelineInfo.pDynamicState			= &dynamicStateCreateInfo;
		pipelineInfo.layout					= m_PipelineLayout;
		pipelineInfo.renderPass				= std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetVulkanRenderPass();
		pipelineInfo.subpass				= 0;
		pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex		= -1; // Optional

		VkPipeline pipeline;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanContext::Get()->GetDevice(), m_PipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

		VulkanGraphicsPipelineInfo	info{};
		info.GraphicsPipeline		= pipeline;
		info.RenderPass				= std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetVulkanRenderPass();
		info.Pass					= pass;

		m_GraphicsPipelines.push_back(info);

		return pipeline;
    }
}
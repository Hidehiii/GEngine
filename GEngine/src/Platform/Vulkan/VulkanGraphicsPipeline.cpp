#include "GEpch.h"
#include "VulkanGraphicsPipeline.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Renderer/Renderer.h"

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

    void VulkanGraphicsPipeline::PrepareRender()
    {
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentCommandBuffer(), "There is no commandbuffer be using");
		GE_CORE_ASSERT(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer(), "There is no framebuffer be using");

		m_Material->Update();

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
        vkCmdBindPipeline(VulkanContext::Get()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, FindOrCreatePipeline());

		VkViewport			Viewport{};
		Viewport.x			= 0.0f;
		Viewport.y			= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight();
		Viewport.width		= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetWidth();
		Viewport.height		= -(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight());
		Viewport.minDepth	= 0.0f;
		Viewport.maxDepth	= 1.0f;
		vkCmdSetViewport(VulkanContext::Get()->GetCurrentCommandBuffer(), 0, 1, &Viewport);

		VkRect2D											Scissor{};
		Scissor.offset = { 0, 0 };
		Scissor.extent = { (unsigned int)(int)VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetWidth(), (unsigned int)(int)VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight() };
		vkCmdSetScissor(VulkanContext::Get()->GetCurrentCommandBuffer(), 0, 1, &Scissor);

		vkCmdSetLineWidth(VulkanContext::Get()->GetCurrentCommandBuffer(), 1.0f);
		vkCmdSetDepthCompareOp(VulkanContext::Get()->GetCurrentCommandBuffer(), Utils::CompareOPToVkCompareOP(m_Material->GetDepthTestOperation()));
		vkCmdSetDepthWriteEnable(VulkanContext::Get()->GetCurrentCommandBuffer(), m_Material->GetEnableDepthWrite());
		vkCmdSetCullMode(VulkanContext::Get()->GetCurrentCommandBuffer(), Utils::CullModeToVkCullMode(m_Material->GetCullMode()));

		auto offsets = Renderer::GetDynamicUniformBufferOffsets();
		vkCmdBindDescriptorSets(VulkanContext::Get()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, m_Material->GetDescriptorSet(Renderer::GetCurrentFrame()), offsets.size(), offsets.data());
    }

	void VulkanGraphicsPipeline::PrepareRender(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer)
	{
		m_Material->Update();

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
		vkCmdBindPipeline(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, FindOrCreatePipeline());

		VkViewport			Viewport{};
		Viewport.x			= 0.0f;
		Viewport.y			= std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetHeight();
		Viewport.width		= std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetWidth();
		Viewport.height		= -(std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetHeight());
		Viewport.minDepth	= 0.0f;
		Viewport.maxDepth	= 1.0f;
		vkCmdSetViewport(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), 0, 1, &Viewport);

		VkRect2D		Scissor{};
		Scissor.offset	= { 0, 0 };
		Scissor.extent	= { (unsigned int)std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetWidth(), (unsigned int)std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetHeight() };
		vkCmdSetScissor(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), 0, 1, &Scissor);

		vkCmdSetLineWidth(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), 1.0f);
		vkCmdSetDepthCompareOp(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), Utils::CompareOPToVkCompareOP(m_Material->GetDepthTestOperation()));
		vkCmdSetDepthWriteEnable(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), m_Material->GetEnableDepthWrite());
		vkCmdSetCullMode(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), Utils::CullModeToVkCullMode(m_Material->GetCullMode()));

		auto offsets = Renderer::GetDynamicUniformBufferOffsets();
		vkCmdBindDescriptorSets(static_cast<VulkanCommandBuffer*>(cmdBuffer)->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, m_Material->GetDescriptorSet(Renderer::GetCurrentFrame()), offsets.size(), offsets.data());
	}

	void VulkanGraphicsPipeline::Render(uint32_t instanceCount, uint32_t indexCount)
	{
		PrepareRender();
		m_VertexBuffer->Bind();
		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexBuffer()->GetCount();
		if (m_VertexBuffer->IsInstanceRendering())
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				RenderCommand::DrawTriangleInstance(indexCount, instanceCount);
				break;
			}
			case VertexTopology::Line:
			{
				RenderCommand::DrawLines(indexCount);
				break;
			}
			case VertexTopology::Point:
			{
				RenderCommand::DrawPoints(indexCount);
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknow type");
				break;
			}
		}
		else
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				RenderCommand::DrawTriangles(indexCount);
				break;
			}
			case VertexTopology::Line:
			{
				RenderCommand::DrawLines(indexCount);
				break;
			}
			case VertexTopology::Point:
			{
				RenderCommand::DrawPoints(indexCount);
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknow type");
				break;
			}
		}
	}

	void VulkanGraphicsPipeline::Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, uint32_t instanceCount, uint32_t indexCount)
	{
		PrepareRender(cmdBuffer, frameBuffer);
		m_VertexBuffer->Bind(cmdBuffer);
		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexBuffer()->GetCount();
		if (m_VertexBuffer->IsInstanceRendering())
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				RenderCommand::DrawTriangleInstance(cmdBuffer, indexCount, instanceCount);
				break;
			}
			case VertexTopology::Line:
			{
				RenderCommand::DrawLines(cmdBuffer, indexCount);
				break;
			}
			case VertexTopology::Point:
			{
				RenderCommand::DrawPoints(cmdBuffer, indexCount);
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknow type");
				break;
			}
		}
		else
		{
			switch (m_VertexBuffer->GetVertexTopologyType())
			{
			case VertexTopology::Triangle:
			{
				RenderCommand::DrawTriangles(cmdBuffer, indexCount);
				break;
			}
			case VertexTopology::Line:
			{
				RenderCommand::DrawLines(cmdBuffer, indexCount);
				break;
			}
			case VertexTopology::Point:
			{
				RenderCommand::DrawPoints(cmdBuffer, indexCount);
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknow type");
				break;
			}
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
    VkPipeline VulkanGraphicsPipeline::FindOrCreatePipeline()
    {

		for (int i = m_GraphicsPipelines.size() - 1; i >= 0; i--)
		{
			if (m_GraphicsPipelines.at(i).RenderPass == VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetVulkanRenderPass())
			{
				return m_GraphicsPipelines.at(i).GraphicsPipeline;
			}
		}
		/*for (int i = m_GraphicsPipelines.size() - 1; i >= 0; i--)
		{
			if (m_GraphicsPipelines.at(i).RenderPass == std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetVulkanRenderPass())
			{
				return m_GraphicsPipelines.at(i).GraphicsPipeline;
			}
		}*/


		// TODO 
		std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->CreateShaderModule();

		std::string shaderMainFuncName = m_Material->GetShader()->GetShaderMainFuncName().c_str();
		std::vector<VkPipelineShaderStageCreateInfo>		ShaderStages;

		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Vertex))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_VERTEX_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Vertex),
				shaderMainFuncName.c_str()));
		}
		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Fragment))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Fragment),
				shaderMainFuncName.c_str()));
		}
		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::TessellationControl))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::TessellationControl),
				shaderMainFuncName.c_str()));
		}
		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::TessellationEvaluation))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::TessellationEvaluation),
				shaderMainFuncName.c_str()));
		}
		if (std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Geometry))
		{
			ShaderStages.push_back(Utils::CreatePipelineShaderStage(VK_SHADER_STAGE_GEOMETRY_BIT,
				std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->GetShaderModule(ShaderStage::Geometry),
				shaderMainFuncName.c_str()));
		}

		

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
		Rasterizer.cullMode									= Utils::CullModeToVkCullMode(m_Material->GetCullMode());
		Rasterizer.frontFace								= VK_FRONT_FACE_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo				Multisampling{};
		Multisampling.sType									= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		Multisampling.sampleShadingEnable					= VK_FALSE;
		Multisampling.rasterizationSamples					= Utils::SampleCountToVulkanFlag(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetSpecification().Samples);
		//Multisampling.rasterizationSamples					= Utils::SampleCountToVulkanFlag(frameBuffer->GetSpecification().Samples);
		Multisampling.minSampleShading						= 1.0f;
		Multisampling.pSampleMask							= nullptr;
		Multisampling.alphaToCoverageEnable					= VK_FALSE;
		Multisampling.alphaToOneEnable						= VK_FALSE;

		VkPipelineColorBlendAttachmentState					ColorBlendAttachment{};
		ColorBlendAttachment.colorWriteMask					= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachment.blendEnable					= VK_TRUE;
		ColorBlendAttachment.srcColorBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendColorSourceFactor());
		ColorBlendAttachment.dstColorBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendColorDestinationFactor());
		ColorBlendAttachment.colorBlendOp					= VK_BLEND_OP_ADD;
		switch (m_Material->GetBlendModeColor())
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
		ColorBlendAttachment.srcAlphaBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendAlphaSourceFactor());
		ColorBlendAttachment.dstAlphaBlendFactor			= Utils::BlendFactorToVulkanBlendFactor(m_Material->GetBlendAlphaDestinationFactor());
		ColorBlendAttachment.alphaBlendOp					= VK_BLEND_OP_ADD;
		switch (m_Material->GetBlendModeAlpha())
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

		std::vector<VkPipelineColorBlendAttachmentState>	attachmentsBlend(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetColorAttachmentCount(), ColorBlendAttachment);
		//std::vector<VkPipelineColorBlendAttachmentState>	attachmentsBlend(frameBuffer->GetColorAttachmentCount(), ColorBlendAttachment);

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
		depthStencil.depthWriteEnable			= m_Material->GetEnableDepthWrite() ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp				= Utils::CompareOPToVkCompareOP(m_Material->GetDepthTestOperation());
		depthStencil.depthBoundsTestEnable		= VK_FALSE;
		depthStencil.minDepthBounds				= 0.0f; // Optional
		depthStencil.maxDepthBounds				= 1.0f; // Optional
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
		pipelineInfo.renderPass				= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetVulkanRenderPass();
		//pipelineInfo.renderPass				= std::static_pointer_cast<VulkanFrameBuffer>(frameBuffer)->GetVulkanRenderPass();
		pipelineInfo.subpass				= 0;
		pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex		= -1; // Optional

		VkPipeline pipeline;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanContext::Get()->GetDevice(), m_PipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

		// TODO 
		std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->DestroyShaderModule();

		VulkanGraphicsPipelineInfo	info{};
		info.GraphicsPipeline		= pipeline;
		info.RenderPass				= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetVulkanRenderPass();

		m_GraphicsPipelines.push_back(info);

		return pipeline;
    }
}
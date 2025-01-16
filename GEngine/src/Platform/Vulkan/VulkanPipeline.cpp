#include "GEpch.h"
#include "VulkanPipeline.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanStorageImage2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "GEngine/Renderer/RenderCommand.h"


#include "Platform/Vulkan/VulkanContext.h"
namespace GEngine
{

    VulkanPipeline::VulkanPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
    {
        m_Material              = std::dynamic_pointer_cast<VulkanMaterial>(material);
        m_VertexBuffer          = std::dynamic_pointer_cast<VulkanVertexBuffer>(vertexBuffer);

		VkPipelineCacheCreateInfo			pipelineCacheInfo{};
		pipelineCacheInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheInfo.initialDataSize	= 0;
		pipelineCacheInfo.pInitialData		= nullptr;

		VK_CHECK_RESULT(vkCreatePipelineCache(VulkanContext::Get()->GetDevice(), &pipelineCacheInfo, nullptr, &m_PipelineCache));
    }

    VulkanPipeline::~VulkanPipeline()
    {
		if (VulkanContext::Get()->GetDevice())
		{
			vkDestroyPipelineCache(VulkanContext::Get()->GetDevice(), m_PipelineCache, nullptr);
			vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_GraphicsPipeline, nullptr);
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), 1, &m_DescriptorSet);
		}
        
    }

    void VulkanPipeline::PrepareRender()
    {
		GE_CORE_ASSERT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), "There is no commandbuffer be using");
		GE_CORE_ASSERT(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer(), "There is no framebuffer be using");

		m_Material->UploadData();

		if (m_FirstCreatePipeline)
		{
			CreateDescriptorSetAndLayout();
			CreatePipeline();
			m_FirstCreatePipeline = false;
		}
		if (m_RecreatePipeline || true)
		{
			vkDestroyPipeline(VulkanContext::Get()->GetDevice(), m_GraphicsPipeline, nullptr);
			vkDestroyPipelineLayout(VulkanContext::Get()->GetDevice(), m_PipelineLayout, nullptr);
			vkFreeDescriptorSets(VulkanContext::Get()->GetDevice(), VulkanContext::Get()->GetDescriptorPool(), 1, &m_DescriptorSet);
			CreateDescriptorSetAndLayout();
			CreatePipeline();
		}
		// 现在是低效率的每帧更新
		UpdateDescriptorSet();
		
		
        vkCmdBindPipeline(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		//VulkanContext::Get()->GetVulkanFunctionEXT().vkCmdSetRasterizationSamplesEXT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), Utils::SampleCountToVulkanFlag(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetSpecification().Samples));

		VkViewport			Viewport{};
		Viewport.x			= 0.0f;
		Viewport.y			= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight();
		Viewport.width		= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetWidth();
		Viewport.height		= -(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight());
		Viewport.minDepth	= 0.0f;
		Viewport.maxDepth	= 1.0f;
		vkCmdSetViewport(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0, 1, &Viewport);

		VkRect2D											Scissor{};
		Scissor.offset = { 0, 0 };
		Scissor.extent = { (unsigned int)(int)VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetWidth(), (unsigned int)(int)VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetHeight() };
		vkCmdSetScissor(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0, 1, &Scissor);
		//VulkanContext::Get()->GetVulkanFunctionEXT().vkCmdSetRasterizationSamplesEXT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), Utils::SampleCountToVulkanFlag(VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetSamples()));
		VkColorComponentFlags colorMaks[] = { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
		//VulkanContext::Get()->GetVulkanFunctionEXT().vkCmdSetColorWriteMaskEXT(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), 0, 1, colorMaks);
		vkCmdSetDepthCompareOp(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), Utils::CompareOPToVkCompareOP(m_Material->GetDepthTestOperation()));
		vkCmdSetDepthWriteEnable(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), m_Material->GetEnableDepthWrite());
		vkCmdSetCullMode(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), Utils::CullModeToVkCullMode(m_Material->GetCullMode()));

		vkCmdBindDescriptorSets(VulkanContext::Get()->GetCurrentDrawCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
    }

	void VulkanPipeline::Render(uint32_t instanceCount, uint32_t indexCount)
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

	void VulkanPipeline::SetVertexBuffer(Ref<VertexBuffer>& buffer)
	{
		m_VertexBuffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(buffer);
		m_RecreatePipeline = true;
	}

	void VulkanPipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<VulkanMaterial>(material);
		m_RecreatePipeline = true;
	}
	void VulkanPipeline::CreateDescriptorSetAndLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding>	layoutBindings;
		// 公共ubo
		std::vector<VulkanUniformBuffer*> publicUniformBuffer		= VulkanUniformBuffer::GetPublicUniformBuffer();
		for (auto buffer : publicUniformBuffer)
		{
			layoutBindings.push_back(buffer->GetDescriptorSetLayoutBinding());
		}
		// 材质ubo
		layoutBindings.push_back(m_Material->GetUniformBuffer()->GetDescriptorSetLayoutBinding());
		// 贴图绑定
		auto texture2Ds		= m_Material->GetTexture2Ds();
		for (auto& texture2D : texture2Ds)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= texture2D.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS;

			layoutBindings.push_back(layoutBinding);
		}
		// storage image 绑定
		auto storageImage2Ds = m_Material->GetStorageImage2Ds();
		for (auto& image2D : storageImage2Ds)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= image2D.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS;

			layoutBindings.push_back(layoutBinding);
		}
		// storage buffer 绑定
		auto storageBuffers = m_Material->GetStorageBuffers();
		for (auto& buffer : storageBuffers)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= buffer.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS;

			layoutBindings.push_back(layoutBinding);
		}
		//cube map 绑定
		auto cubeMaps = m_Material->GetCubeMaps();
		for (auto& cubeMap : cubeMaps)
		{
			VkDescriptorSetLayoutBinding		layoutBinding{};
			layoutBinding.binding				= cubeMap.Slot;
			layoutBinding.descriptorCount		= 1;
			layoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.pImmutableSamplers	= nullptr;
			layoutBinding.stageFlags			= VK_SHADER_STAGE_ALL_GRAPHICS;

			layoutBindings.push_back(layoutBinding);
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
		// 更新贴图绑定
		{
			auto texture2Ds					= m_Material->GetTexture2Ds();
			for (auto& texture2D : texture2Ds)
			{
				VkDescriptorImageInfo			imageInfo = std::dynamic_pointer_cast<VulkanTexture2D>(texture2D.Texture)->GetDescriptorImageInfo();
				VkWriteDescriptorSet			descriptorWrite{};
				descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet			= m_DescriptorSet;
				descriptorWrite.dstBinding		= texture2D.Slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo		= nullptr;
				descriptorWrite.pImageInfo		= &imageInfo; 
				descriptorWrite.pTexelBufferView = nullptr; // Optional

				vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
		// 更新cubeMap绑定
		{
			auto cubeMaps						= m_Material->GetCubeMaps();
			for (auto& cubeMap : cubeMaps)
			{
				VkDescriptorImageInfo			imageInfo = std::dynamic_pointer_cast<VulkanCubeMap>(cubeMap.Cubemap)->GetDescriptorImageInfo();
				VkWriteDescriptorSet			descriptorWrite{};
				descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet			= m_DescriptorSet;
				descriptorWrite.dstBinding		= cubeMap.Slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo		= nullptr;
				descriptorWrite.pImageInfo		= &imageInfo;
				descriptorWrite.pTexelBufferView = nullptr; // Optional

				vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
		// 更新storage image绑定
		{
			auto storageImage2Ds = m_Material->GetStorageImage2Ds();
			for (auto& image2D : storageImage2Ds)
			{
				VkDescriptorImageInfo			imageInfo = std::dynamic_pointer_cast<VulkanStorageImage2D>(image2D.Image)->GetDescriptorImageInfo();
				VkWriteDescriptorSet			descriptorWrite{};
				descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet			= m_DescriptorSet;
				descriptorWrite.dstBinding		= image2D.Slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo		= nullptr;
				descriptorWrite.pImageInfo		= &imageInfo;
				descriptorWrite.pTexelBufferView = nullptr; // Optional

				vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
		// 更新storage buffer绑定
		{
			auto storageBuffers = m_Material->GetStorageBuffers();
			for (auto& buffer : storageBuffers)
			{
				VkDescriptorBufferInfo			bufferInfo = std::dynamic_pointer_cast<VulkanStorageBuffer>(buffer.Buffer)->GetDescriptorBufferInfo();
				VkWriteDescriptorSet			descriptorWrite{};
				descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet			= m_DescriptorSet;
				descriptorWrite.dstBinding		= buffer.Slot;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo		= &bufferInfo;
				descriptorWrite.pImageInfo		= nullptr;
				descriptorWrite.pTexelBufferView = nullptr; // Optional

				vkUpdateDescriptorSets(VulkanContext::Get()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}
	}
    void VulkanPipeline::CreatePipeline()
    {
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
		pipelineLayoutInfo.pSetLayouts				= &m_DescriptorSetLayout;
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
		pipelineInfo.renderPass				= VulkanFrameBuffer::GetCurrentVulkanFrameBuffer()->GetRenderPass();
		pipelineInfo.subpass				= 0;
		pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex		= -1; // Optional


		

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanContext::Get()->GetDevice(), m_PipelineCache, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

		// TODO 
		std::dynamic_pointer_cast<VulkanShader>(m_Material->GetShader())->DestroyShaderModule();
    }
}
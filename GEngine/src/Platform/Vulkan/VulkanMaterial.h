#pragma once
#include "GEngine/Graphics/Material.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	class GENGINE_API VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(const Ref<Shader>& shader, const std::string& name = "");
		virtual ~VulkanMaterial() override;
		
		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual Buffer SetUniformBuffer(const int& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf) override;

		virtual std::vector<uint32_t> GetDynamicOffsets(const int& pass);

		std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>>	GetUniformBuffer(const int& pass)					{ return m_UniformBuffers.at(pass); }
		VkDescriptorSetLayout*									GetDescriptorSetLayout(const int& pass)				{ return &m_DescriptorSetLayouts.at(pass); }
		VkDescriptorSet*										GetDescriptorSet(const int& pass, const int& index) { return &m_DescriptorSets.at(pass * Graphics::GetFrameCount() + index); }
		VkPipelineLayout&										GetPipelineLayout(const int& pass)					{ return m_PipelineLayouts.at(pass); }
	protected:
		virtual void Update(CommandBuffer* cmdBuffer, const int& pass) override;

		virtual void ResourceUpdateNotify() override { m_NeedUpdateDescripotrSetFrames = std::vector<uint8_t>(std::pow(2, Graphics::GetFrameCount()) - 1, m_Passes.size()); }
	private:
		void CreateDescriptorAndLayout();
		void UpdateDescriptorSet(const int& pass, const int& index);
		void UpdateDynamicOffsets(const int& pass);
	private:
		Ref<VulkanShader>														m_Shader;
		std::vector<std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>>>		m_UniformBuffers; // pass { bind point : buffer }
		std::vector<std::vector<uint32_t>>										m_DynamicUniformBufferOffsets; // pass { offsets }
		
		std::vector<VkDescriptorSet>											m_DescriptorSets;
		
		std::vector<uint8_t>													m_NeedUpdateDescripotrSetFrames;

		friend class VulkanGraphicsPipeline;
	};

}


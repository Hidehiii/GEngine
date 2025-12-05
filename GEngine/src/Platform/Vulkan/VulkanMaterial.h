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

		virtual void Update(CommandBuffer* cmdBuffer, const int& pass) override;

		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetShader(const Ref<Shader>& shader) override;

		virtual Buffer SetUniformBuffer(const int& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf) override;

		virtual std::vector<uint32_t> GetDynamicOffsets(const int& pass) { return m_DynamicUniformBufferOffsets.at(pass); }

		std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>>	GetUniformBuffer(const int& pass)					{ return m_UniformBuffers.at(pass); }
		VkDescriptorSetLayout*									GetDescriptorSetLayout(const int& pass)				{ return &m_DescriptorSetLayouts.at(pass); }
		VkDescriptorSet*										GetDescriptorSet(const int& pass, const int& index) { return &m_DescriptorSets.at(pass * Graphics::GetFrameCount() + index); }
	private:
		// 理论上只要后面不改动ubo和贴图只需要创建和更新一次
		void CreateDescriptorSetAndLayout();
		void UpdateDescriptorSet(const int& pass, const int& index);
		void UpdateDynamicOffsets(const int& pass);
	private:
		Ref<VulkanShader>														m_Shader;
		std::vector<std::unordered_map<uint32_t, Ref<VulkanUniformBuffer>>>		m_UniformBuffers;
		std::vector<std::vector<uint32_t>>										m_DynamicUniformBufferOffsets; // pass { offsets }
		std::vector<VkDescriptorSetLayout>										m_DescriptorSetLayouts;
		std::vector<VkDescriptorSet>											m_DescriptorSets;
		uint8_t																	m_NeedUpdateDescripotrSetFrames = std::pow(2, Graphics::GetFrameCount()) - 1;
	};

}


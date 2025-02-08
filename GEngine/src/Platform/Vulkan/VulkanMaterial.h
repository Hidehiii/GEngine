#pragma once
#include "GEngine/Renderer/Material.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
	class GENGINE_API VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(const Ref<Shader>& shader, const std::string& name = "");
		virtual ~VulkanMaterial() override;

		virtual void Update() override;

		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;


		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetShader(const Ref<Shader>& shader) override;

		Ref<VulkanUniformBuffer>	GetUniformBuffer()			{ return m_UniformBuffer; }
		VkDescriptorSetLayout*		GetDescriptorSetLayout()	{ return &m_DescriptorSetLayout; }
		VkDescriptorSet*			GetDescriptorSet(int index) { return &m_DescriptorSets.at(index); }
	private:
		// 理论上只要后面不改动ubo和贴图只需要创建和更新一次
		void CreateDescriptorSetAndLayout();
		void UpdateDescriptorSet(int index);
		void UpdateAllDescriptorSet();
	private:
		Ref<VulkanShader>									m_Shader;
		Ref<VulkanUniformBuffer>							m_UniformBuffer;
		VkDescriptorSetLayout								m_DescriptorSetLayout;
		std::vector<VkDescriptorSet>						m_DescriptorSets;
		uint8_t												m_NeedUpdateDescripotrSetFrames = std::pow(2, Renderer::GetFramesInFlight()) - 1;
	};

}


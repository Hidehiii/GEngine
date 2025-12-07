#include "GEpch.h"
#include "VulkanTexture2DCombineSampler.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace GEngine
{
	VulkanTexture2DCombineSampler::VulkanTexture2DCombineSampler(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler)
	{
		m_Texture = std::dynamic_pointer_cast<VulkanTexture2D>(texture);
		m_Sampler = std::dynamic_pointer_cast<VulkanSampler>(sampler);

		m_ImageInfo.sampler		= m_Sampler->GetVulkanSampler();
		m_ImageInfo.imageView	= m_Texture->GetImageView();
		m_ImageInfo.imageLayout = m_Texture->GetImageLayout();
	}

	void VulkanTexture2DCombineSampler::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
		m_Texture->SetImageLayout(((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView	= m_Texture->GetImageView();

		m_Sampler->Bind(slot);
		m_ImageInfo.sampler		= m_Sampler->GetVulkanSampler();
	}

	void VulkanTexture2DCombineSampler::SetTexture(const Ref<Texture2D>& texture)
	{
		m_Texture				= std::dynamic_pointer_cast<VulkanTexture2D>(texture);
		m_ImageInfo.imageView	= m_Texture->GetImageView();
		m_ImageInfo.imageLayout = m_Texture->GetImageLayout();
	}

	void VulkanTexture2DCombineSampler::SetSampler(const Ref<Sampler>& sampler)
	{
		m_Sampler			= std::dynamic_pointer_cast<VulkanSampler>(sampler);
		m_ImageInfo.sampler = m_Sampler->GetVulkanSampler();
	}
}
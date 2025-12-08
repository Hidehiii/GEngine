#include "GEpch.h"
#include "VulkanCubeMapCombineSampler.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace GEngine
{
	VulkanCubeMapCombineSampler::VulkanCubeMapCombineSampler(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler)
	{
		m_CubeMap = std::dynamic_pointer_cast<VulkanCubeMap>(cubemap);
		m_Sampler = std::dynamic_pointer_cast<VulkanSampler>(sampler);

		m_ImageInfo.sampler		= m_Sampler->GetVulkanSampler();
		m_ImageInfo.imageView	= m_CubeMap->GetImageView();
		m_ImageInfo.imageLayout = m_CubeMap->GetImageLayout();
	}

	void VulkanCubeMapCombineSampler::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
		m_CubeMap->SetImageLayout(((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView	= m_CubeMap->GetImageView();

		m_Sampler->Bind(slot);
		m_ImageInfo.sampler		= m_Sampler->GetVulkanSampler();
	}

	void VulkanCubeMapCombineSampler::SetCubeMap(const Ref<CubeMap>& cubemap)
	{
		m_CubeMap				= std::dynamic_pointer_cast<VulkanCubeMap>(cubemap);
		m_ImageInfo.imageView	= m_CubeMap->GetImageView();
		m_ImageInfo.imageLayout = m_CubeMap->GetImageLayout();
	}

	void VulkanCubeMapCombineSampler::SetSampler(const Ref<Sampler>& sampler)
	{
		m_Sampler			= std::dynamic_pointer_cast<VulkanSampler>(sampler);
		m_ImageInfo.sampler = m_Sampler->GetVulkanSampler();
	}


}
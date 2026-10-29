#include "GEpch.h"
#include "Platform/Vulkan/VulkanTexture2DArray.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace GEngine
{
    VulkanTexture2DArray::VulkanTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
    {
		m_ExclusiveSharing = Utils::UseExclusiveTransferImageSharing();
		m_Height    = height;
		m_Width     = width;
		m_Format    = format;
        m_Layers    = layers;
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		Utils::CreateImages(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			m_Width,
			m_Height,
			Utils::RenderImage2DFormatToVulkanFormat(m_Format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			m_Layers,
			0,
			m_MipLevels,
			m_Image,
			m_ImageAllocation, m_ExclusiveSharing);
		Utils::CreateImageViews(VulkanContext::Get()->GetDevice(), m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format), VK_IMAGE_VIEW_TYPE_2D_ARRAY, m_Layers, m_AspectFlag, m_MipLevels, m_ImageView);
		CreateSampler();
    }
    VulkanTexture2DArray::~VulkanTexture2DArray()
    {
		auto* context = VulkanContext::Get();
		if (context != nullptr && context->GetDevice() != VK_NULL_HANDLE)
		{
			const VkImageView imageView = m_ImageView;
			const VkImage image = m_Image;
			const VmaAllocation imageAllocation = m_ImageAllocation;
			const VkSampler sampler = m_Sampler;
			const VmaAllocator allocator = context->GetVmaAllocator();
			context->RetireResource([imageView, image, imageAllocation, sampler, allocator](VkDevice device)
			{
				if (imageView != VK_NULL_HANDLE)
					vkDestroyImageView(device, imageView, nullptr);
				if (image != VK_NULL_HANDLE)
					vmaDestroyImage(allocator, image, imageAllocation);
				if (sampler != VK_NULL_HANDLE)
					vkDestroySampler(device, sampler, nullptr);
			});
		}
		m_ImageView = VK_NULL_HANDLE;
		m_Image = VK_NULL_HANDLE;
		m_ImageAllocation = nullptr;
		m_Sampler = VK_NULL_HANDLE;
    }
	void VulkanTexture2DArray::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
		SetImageLayout(((VulkanCommandBuffer*)cmdBuffer)->GetCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView	= m_ImageView;
		m_ImageInfo.sampler		= m_Sampler;
	}
	void VulkanTexture2DArray::SetData(const void* data, uint32_t size)
	{
		uint32_t tempSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		for (uint32_t layer = 0; layer < size / tempSize; layer++)
		{
			uint32_t offset = tempSize * layer;
			void* tempData = new std::byte[tempSize];
			memcpy(tempData, ((std::byte*)data) + offset, tempSize);
			SetData(tempData, tempSize, layer);
			delete[](std::byte*)tempData;
		}
	}
	void VulkanTexture2DArray::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer)
	{
		SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		std::dynamic_pointer_cast<VulkanTexture2D>(texture)->SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		Utils::CopyImageToImage(width, height, std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImage(),
			std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetImageLayout(),
			std::dynamic_pointer_cast<VulkanTexture2D>(texture)->GetAspectFlag(),
			0, 0,
			m_Image, m_ImageLayout, m_AspectFlag,
			0, layer);

		if (m_GenerateMipmap)
		{
			Utils::GenerateMipmap(m_Image, m_Width, m_Height, m_MipLevels, m_AspectFlag, 0, m_Layers);
			SetAllSubresourceLayouts(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else
		{
			SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	void VulkanTexture2DArray::SetData(const void* data, uint32_t size, uint32_t layer)
	{
		VkBuffer    buffer;
		VmaAllocation allocation = nullptr;

		Utils::CreateBuffer(VulkanContext::Get()->GetPhysicalDevice(),
			VulkanContext::Get()->GetDevice(),
			static_cast<uint32_t>(size),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer,
			allocation);

		void* tempData = nullptr;
		vmaMapMemory(VulkanContext::Get()->GetVmaAllocator(), allocation, &tempData);
		memcpy(tempData, data, static_cast<uint32_t>(size));
		vmaUnmapMemory(VulkanContext::Get()->GetVmaAllocator(), allocation);
		SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utils::CopyBufferToImage(buffer, m_Image, m_Width, m_Height, 0, layer, m_AspectFlag);

		vmaDestroyBuffer(VulkanContext::Get()->GetVmaAllocator(), buffer, allocation);
		if (m_GenerateMipmap)
		{
			Utils::GenerateMipmap(m_Image, m_Width, m_Height, m_MipLevels, m_AspectFlag, 0, m_Layers);
			SetAllSubresourceLayouts(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else
		{
			SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	void VulkanTexture2DArray::ReadLayerData(uint32_t layer, uint32_t size, void* destination)
	{
		const uint32_t expectedSize = m_Width * m_Height * 4;
		if (!destination || m_Format != RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM || layer >= m_Layers || size != expectedSize)
			throw std::invalid_argument("Vulkan array readback requires an RGBA8 layer and its exact byte size.");
		const VkImageLayout previousLayout = GetSubresourceLayout(0, layer);
		if (previousLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			throw std::invalid_argument("Cannot read an uninitialized Vulkan texture-array layer.");
		auto* context = VulkanContext::Get();
		context->WaitForIdle();
		VkBuffer staging = VK_NULL_HANDLE;
		VmaAllocation stagingAllocation = nullptr;
		try
		{
			Utils::CreateBuffer(context->GetPhysicalDevice(), context->GetDevice(), size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingAllocation);
			const VkCommandBuffer command = context->BeginSingleTimeGraphicsCommand();
			if (previousLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
				Utils::TransitionImageLayout(command, m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format),
					previousLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, m_AspectFlag, 1, 0, layer);
			VkBufferImageCopy region{};
			region.imageSubresource = { m_AspectFlag, 0, layer, 1 };
			region.imageExtent = { m_Width, m_Height, 1 };
			vkCmdCopyImageToBuffer(command, m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging, 1, &region);
			if (previousLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
				Utils::TransitionImageLayout(command, m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format),
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, previousLayout, 1, m_AspectFlag, 1, 0, layer);
			context->EndSingleTimeGraphicsCommand(command);
			void* mapped = nullptr;
			if (vmaMapMemory(context->GetVmaAllocator(), stagingAllocation, &mapped) != VK_SUCCESS)
				throw std::runtime_error("Failed to map Vulkan texture-array readback memory.");
			memcpy(destination, mapped, size);
			vmaUnmapMemory(context->GetVmaAllocator(), stagingAllocation);
		}
		catch (...)
		{
			if (staging != VK_NULL_HANDLE) vmaDestroyBuffer(context->GetVmaAllocator(), staging, stagingAllocation);
			throw;
		}
		vmaDestroyBuffer(context->GetVmaAllocator(), staging, stagingAllocation);
	}
	void VulkanTexture2DArray::SetImageLayout(VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = VulkanContext::Get()->BeginSingleTimeGraphicsCommand();
		SetImageLayout(commandBuffer, newLayout);
		VulkanContext::Get()->EndSingleTimeGraphicsCommand(commandBuffer);
	}
	void VulkanTexture2DArray::SetImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout)
	{
		for (uint32_t layer = 0; layer < m_Layers; ++layer)
			for (uint32_t mip = 0; mip < m_MipLevels; ++mip)
			{
				const auto oldLayout = GetSubresourceLayout(mip, layer);
				if (oldLayout == newLayout) continue;
				Utils::TransitionImageLayout(cmdBuffer, m_Image, Utils::RenderImage2DFormatToVulkanFormat(m_Format),
					oldLayout, newLayout, 1, m_AspectFlag, 1, mip, layer);
				SetSubresourceLayout(mip, layer, newLayout);
			}
	}

	VkImageLayout VulkanTexture2DArray::GetSubresourceLayout(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		if (mipLevel >= m_MipLevels || arrayLayer >= m_Layers)
			throw std::out_of_range("Vulkan texture-array subresource is out of bounds.");
		const size_t index = static_cast<size_t>(arrayLayer) * m_MipLevels + mipLevel;
		return m_SubresourceLayouts.empty() ? m_ImageLayout : m_SubresourceLayouts[index];
	}

	void VulkanTexture2DArray::SetSubresourceLayout(uint32_t mipLevel, uint32_t arrayLayer, VkImageLayout layout)
	{
		if (mipLevel >= m_MipLevels || arrayLayer >= m_Layers)
			throw std::out_of_range("Vulkan texture-array subresource is out of bounds.");
		const size_t count = static_cast<size_t>(m_MipLevels) * m_Layers;
		if (m_SubresourceLayouts.empty()) m_SubresourceLayouts.assign(count, m_ImageLayout);
		m_SubresourceLayouts[static_cast<size_t>(arrayLayer) * m_MipLevels + mipLevel] = layout;
		m_ImageLayout = std::all_of(m_SubresourceLayouts.begin(), m_SubresourceLayouts.end(),
			[layout](VkImageLayout current) { return current == layout; }) ? layout : VK_IMAGE_LAYOUT_UNDEFINED;
	}

	void VulkanTexture2DArray::SetAllSubresourceLayouts(VkImageLayout layout)
	{
		m_SubresourceLayouts.assign(static_cast<size_t>(m_MipLevels) * m_Layers, layout);
		m_ImageLayout = layout;
	}

	uint8_t VulkanTexture2DArray::GetQueueOwnership(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		if (mipLevel >= m_MipLevels || arrayLayer >= m_Layers)
			throw std::out_of_range("Vulkan texture-array subresource is out of bounds.");
		return m_QueueOwnership.empty() ? 0 : m_QueueOwnership[static_cast<size_t>(arrayLayer) * m_MipLevels + mipLevel];
	}

	void VulkanTexture2DArray::SetQueueOwnership(uint32_t mipLevel, uint32_t arrayLayer, uint8_t ownership)
	{
		if (mipLevel >= m_MipLevels || arrayLayer >= m_Layers)
			throw std::out_of_range("Vulkan texture-array subresource is out of bounds.");
		if (m_QueueOwnership.empty()) m_QueueOwnership.assign(static_cast<size_t>(m_MipLevels) * m_Layers, 0);
		m_QueueOwnership[static_cast<size_t>(arrayLayer) * m_MipLevels + mipLevel] = ownership;
	}
	void VulkanTexture2DArray::CreateSampler()
	{
		VkSamplerCreateInfo             samplerInfo{};
		samplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter			= VK_FILTER_LINEAR;
		samplerInfo.minFilter			= VK_FILTER_LINEAR;
		samplerInfo.addressModeU		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable	= VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanContext::Get()->GetPhysicalDevice(), &properties);

		samplerInfo.maxAnisotropy			= properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias				= 0.0f;
		samplerInfo.minLod					= 0.0f;
		samplerInfo.maxLod					= VK_LOD_CLAMP_NONE;

		VK_CHECK_RESULT(vkCreateSampler(VulkanContext::Get()->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
	}
}

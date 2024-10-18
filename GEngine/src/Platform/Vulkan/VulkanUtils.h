#pragma once
#include <vulkan/vulkan.h>


#ifdef GE_PLATFORM_WINDOWS
#define VK_CHECK_RESULT(f)																								\
	{																													\
		VkResult res = f;																								\
		GE_CORE_ASSERT(res == VK_SUCCESS, "Fatal : VkResult is {0}, In File : {1}, At Line : {2}, Statement : {4}",		\
						res, __FILE__, __LINE__, VAR_NAME(f));															\
	}
#endif

namespace GEngine
{
	namespace Utils
	{
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDvice);
		void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	}
	
}


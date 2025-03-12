#pragma once
#include "GEngine/Renderer/CommandBuffer.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include <vulkan/vulkan.h>
#include <optional>
namespace GEngine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;
		std::optional<uint32_t>	ComputeFamily;

		bool IsComplete()
		{
			return GraphicsFamily.has_value() && PresentFamily.has_value() && ComputeFamily.has_value();
		}

	};

	class GENGINE_API VulkanCommandBufferPool
	{
	public:
		VulkanCommandBufferPool() = default;
		VulkanCommandBufferPool(QueueFamilyIndices queueFamilyIndices, int count = 10);
		~VulkanCommandBufferPool();
		void Release();

		VkCommandBuffer GetGraphicsCommandBuffer(int index = 0) { return m_GraphicsCommandBuffers.at(index); }
		VkCommandBuffer GetComputeCommandBuffer(int index = 0) { return m_ComputeCommandBuffers.at(index); }
		VkCommandBuffer GetSecondaryCommandBuffer(int index = 0) { return m_SecondaryCommandBuffers.at(index); }
		uint32_t		GetGraphicsCommandBuffersSize() { return m_GraphicsCommandBuffers.size(); }
		uint32_t		GetSecondaryCommandBuffersSize() { return m_SecondaryCommandBuffers.size(); }

		VkCommandBuffer BeginSingleTimeGraphicsCommand();
		void			EndSingleTimeGraphicsCommand(VkCommandBuffer commandBuffer);
		VkCommandBuffer BeginSingleTimeComputeCommand();
		void			EndSingleTimeComputeCommand(VkCommandBuffer commandBuffer);
	private:
		void			CreateCommandPool(QueueFamilyIndices queueFamilyIndices);
	private:
		VkCommandPool					m_GraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool					m_ComputeCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer>	m_GraphicsCommandBuffers;
		std::vector<VkCommandBuffer>	m_ComputeCommandBuffers;
		std::vector<VkCommandBuffer>	m_SecondaryCommandBuffers;
	};


	class GENGINE_API VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VkCommandBuffer buffer, CommandBufferType type, VkSemaphore semaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
		virtual ~VulkanCommandBuffer();

		virtual void Render(Ref<Scene>&scene) override;
		virtual void Render(Ref<GraphicsPipeline>&pipeline, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

		virtual void Compute(Ref<ComputePipeline>&pipeline, uint32_t x, uint32_t y, uint32_t z) override;


		VkCommandBuffer GetCommandBuffer() { return m_CommandBuffer; }
		VkSemaphore		GetSemaphore() { return m_Semaphore; }
		VkFence			GetFence() { return m_Fence; }
	protected:
		virtual void Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera);
		virtual void Begin(Ref<FrameBuffer>& buffer, const Camera& camera);

		virtual void End() override;
	private:
		VkCommandBuffer				m_CommandBuffer;
		Ref<VulkanFrameBuffer>		m_FrameBuffer;
		VkSemaphore					m_Semaphore;
		VkFence						m_Fence;

		friend class VulkanRendererAPI;
	};
}



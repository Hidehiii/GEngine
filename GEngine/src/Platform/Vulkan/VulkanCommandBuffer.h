#pragma once
#include "GEngine/Renderer/CommandBuffer.h"
#include "GEngine/Renderer/Renderer.h"
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
		uint32_t		GetCommandBuffersSize() { return m_GraphicsCommandBuffers.size(); }
		uint32_t		GetSecondaryCommandBuffersSize() { return m_SecondaryCommandBuffers.size(); }

		VkCommandBuffer BeginSingleTimeGraphicsCommands();
		void			EndSingleTimeGraphicsCommands(VkCommandBuffer commandBuffer, VkQueue queue);
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
		VulkanCommandBuffer(VkCommandBuffer buffer);
		virtual ~VulkanCommandBuffer();

		virtual void Begin(Ref<FrameBuffer>&buffer, const Editor::EditorCamera & camera, std::vector<CommandBuffer*> waitBuffers) override;
		virtual void Begin(Ref<FrameBuffer>&buffer, const Camera & camera, std::vector<CommandBuffer*> waitBuffers) override;

		virtual void End() override;

		virtual void Render(Ref<Scene>&scene) override;
		virtual void Render(Ref<GraphicsPipeline>&pipeline) override;

		virtual void Compute(Ref<ComputePipeline>&pipeline) override;


		VkCommandBuffer GetCommandBuffer() { return m_CommandBuffer; }
		VkSemaphore		GetSemaphore() { return m_Semaphores.at(Renderer::GetCurrentFrame()); }
	private:
		VkCommandBuffer				m_CommandBuffer;
		Ref<FrameBuffer>			m_FrameBuffer;
		std::vector<VkSemaphore>	m_Semaphores;
	};
}



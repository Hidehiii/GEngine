#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include "GEngine/Graphics/GraphicsResource.h"
#include "GEngine/Graphics/RenderDevice.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/VertexBuffer.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include "Material.h"

namespace GEngine
{
	class CommandBuffer;
	class FrameBuffer;

	struct GraphicsCapabilities
	{
		bool RenderPass = false;
		bool FrameBuffer = false;
		bool Texture2D = false;
		bool Texture2DArray = false;
		bool CubeMap = false;
		bool Sampler = false;
		bool UniformBuffer = false;
		bool StorageBuffer = false;
		bool StorageImage = false;
		bool Compute = false;
		bool Subpasses = false;
	};

	class GENGINE_API GraphicsAPI : public RenderDevice
	{
	public:
		GraphicsAPI()
		{
			m_GraphicsQueue = CreateScope<GraphicsQueue>(COMMAND_BUFFER_TYPE_GRAPHICS,
				[this](const Ref<CommandBuffer>& commandBuffer) { SubmitCommandBuffer(commandBuffer); },
				[this]() { WaitForIdle(); });
			m_ComputeQueue = CreateScope<GraphicsQueue>(COMMAND_BUFFER_TYPE_COMPUTE,
				[this](const Ref<CommandBuffer>& commandBuffer) { SubmitCommandBuffer(commandBuffer); },
				[this]() { WaitForIdle(); });
			m_TransferQueue = CreateScope<GraphicsQueue>(COMMAND_BUFFER_TYPE_TRANSFER,
				[this](const Ref<CommandBuffer>& commandBuffer) { SubmitCommandBuffer(commandBuffer); },
				[this]() { WaitForIdle(); });
		}

		GraphicsQueue& GetQueue(CommandBufferType type) override
		{
			switch (type)
			{
			case COMMAND_BUFFER_TYPE_GRAPHICS: return *m_GraphicsQueue;
			case COMMAND_BUFFER_TYPE_COMPUTE: return *m_ComputeQueue;
			case COMMAND_BUFFER_TYPE_TRANSFER: return *m_TransferQueue;
			default:
				GE_CORE_ASSERT(false, "A valid queue type is required.");
				return *m_GraphicsQueue;
			}
		}

		virtual std::vector<std::string> GetExtensions() = 0;
		virtual GraphicsCapabilities GetCapabilities() const = 0;

		virtual uint32_t GetMaxTexture2DSize() = 0;
		virtual uint32_t GetMaxCombinedTextureCount() = 0;
		virtual uint32_t GetMaxPerStageTextureCount() = 0;
		virtual uint32_t GetMaxTextureArrayLayers() = 0;

		virtual uint32_t GetMinUniformBufferOffsetAlignment() = 0;
		virtual uint32_t GetMaxUniformBufferSize() = 0;

		virtual Ref<CommandBuffer> GetGraphicsCommandBuffer() = 0;
		virtual Ref<CommandBuffer> GetComputeCommandBuffer() = 0;

		// compute
		virtual Vector3 GetMaxComputeWorkGroupCount() = 0;
		virtual Vector3 GetMaxComputeWorkGroupSize() = 0;
		virtual uint32_t GetMaxComputeWorkGroupInvocations() = 0;

		virtual void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second) = 0;
		virtual void SubmitCommandBuffer(const Ref<CommandBuffer>& commandBuffer) = 0;
		virtual void WaitForIdle() = 0;
		virtual void TransitionResource(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsResource>& resource,
			GraphicsResourceState before, GraphicsResourceState after) = 0;


		inline static Graphics_API GetAPI() { return s_API; }
	protected:
		void* GetNativeResource(const Ref<GraphicsResource>& resource) const
		{
			return resource ? resource->GetNativeResource() : nullptr;
		}

		inline static Graphics_API s_API = GRAPHICS_API_NONE;

	private:
		Scope<GraphicsQueue> m_GraphicsQueue;
		Scope<GraphicsQueue> m_ComputeQueue;
		Scope<GraphicsQueue> m_TransferQueue;
	};

}




#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

#include <functional>
#include <utility>

namespace GEngine
{
	class CommandBuffer;

	class GENGINE_API GraphicsQueue
	{
	public:
		using SubmitCallback = std::function<void(const Ref<CommandBuffer>&)>;
		using WaitForIdleCallback = std::function<void()>;

		GraphicsQueue(CommandBufferType type, SubmitCallback submit, WaitForIdleCallback waitForIdle)
			: m_Type(type), m_Submit(std::move(submit)), m_WaitForIdle(std::move(waitForIdle))
		{
		}

		CommandBufferType GetType() const { return m_Type; }

		void Submit(const Ref<CommandBuffer>& commandBuffer) const
		{
			GE_CORE_ASSERT(commandBuffer != nullptr, "A command buffer is required for queue submission.");
			m_Submit(commandBuffer);
		}

		void WaitForIdle() const
		{
			m_WaitForIdle();
		}

	private:
		CommandBufferType m_Type = COMMAND_BUFFER_TYPE_NONE;
		SubmitCallback m_Submit;
		WaitForIdleCallback m_WaitForIdle;
	};
}

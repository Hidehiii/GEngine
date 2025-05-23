#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "GEngine/Compute/ComputePipeline.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Components/Camera/Camera.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include <set>

namespace GEngine
{
	enum class CommandBufferType
	{
		None,
		Graphics,
		Compute,
		Transfer,
	};

	class GENGINE_API CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Render(Ref<GraphicsPipeline>& pipeline, std::string pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) = 0;

		virtual void Compute(Ref<ComputePipeline>& pipeline, std::string pass, uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual CommandBufferType				GetType() { return m_Type; }

		virtual void Begin(Ref<FrameBuffer>& buffer) = 0;
		virtual void End() = 0;
	protected:
		CommandBufferType				m_Type = CommandBufferType::None;
	};
}



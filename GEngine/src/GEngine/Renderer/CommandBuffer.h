#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Renderer/GraphicsPipeline.h"
#include "GEngine/Compute/ComputePipeline.h"
#include "GEngine/Renderer/FrameBuffer.h"
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
	};

	class GENGINE_API CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Render(Ref<Scene>& scene) = 0;
		virtual void Render(Ref<GraphicsPipeline>& pipeline, uint32_t instanceCount = 1, uint32_t indexCount = 0) = 0;

		virtual void Compute(Ref<ComputePipeline>& pipeline, uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual void						AddWaitCommand(CommandBuffer* command) { m_WaitCommands.insert(command); }
		virtual std::set<CommandBuffer*>	GetWaitCommands() { return m_WaitCommands; }
		virtual void						ClearWaitCommands() { m_WaitCommands.clear(); }
		virtual CommandBufferType			GetType() { return m_Type; }
	protected:
		virtual void Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera) = 0;
		virtual void Begin(Ref<FrameBuffer>& buffer, const Camera& camera) = 0;

		virtual void End() = 0;
	protected:
		std::set<CommandBuffer*>		m_WaitCommands;
		CommandBufferType				m_Type = CommandBufferType::None;
	};
}



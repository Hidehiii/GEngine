#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Renderer/GraphicsPipeline.h"
#include "GEngine/Compute/ComputePipeline.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "GEngine/Components/Camera/Camera.h"
#include "GEngine/Components/Camera/EditorCamera.h"

namespace GEngine
{
	class GENGINE_API CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera) = 0;
		virtual void Begin(Ref<FrameBuffer>& buffer, const Camera& camera) = 0;

		virtual void End() = 0;

		virtual void Render(Ref<Scene>& scene) = 0;
		virtual void Render(Ref<GraphicsPipeline>& pipeline, uint32_t instanceCount = 1, uint32_t indexCount = 0) = 0;

		virtual void Compute(Ref<ComputePipeline>& pipeline, uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual void						SetWaitCommands(std::vector<CommandBuffer*> commands) { m_WaitCommands = commands; }
		virtual std::vector<CommandBuffer*>	GetWaitCommands() { return m_WaitCommands; }
	protected:
		std::vector<CommandBuffer*>		m_WaitCommands;
	};
}



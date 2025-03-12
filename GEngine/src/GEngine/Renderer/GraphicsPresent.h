#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Math/Math.h"
#include <set>

namespace GEngine
{
	class CommandBuffer;
	class Scene;
	class GraphicsPipeline;
	class ComputePipeline;

	class GENGINE_API GraphicsPresent
	{
	public:
		GraphicsPresent() {}
		virtual bool AquireImage() { ClearWaitCommands(); return true; }
		virtual void Begin() {}
		virtual void End() {}

		static CommandBuffer* GetCommandBuffer();
		static void Render(Ref<Scene>& scene);
		static void Render(Ref<GraphicsPipeline>& pipeline, uint32_t instanceCount = 1, uint32_t indexCount = 0);
		static void Compute(Ref<ComputePipeline>& pipeline, uint32_t x, uint32_t y, uint32_t z);
		static void AddWaitCommand(CommandBuffer* cmd) { s_WaitCommands.insert(cmd); }
		static void SetWaitCommands(std::set<CommandBuffer*> cmds) { s_WaitCommands = cmds; }
		static void ClearWaitCommands() { s_WaitCommands.clear(); }
	protected:
		void OnWindowResize(const Vector2 size) { m_WindowResize = size; }

		friend class Application;

		static Scope<GraphicsPresent> Create();
	protected:
		Vector2								m_WindowResize = Vector2(0.0f, 0.0f);

		static std::set<CommandBuffer*>		s_WaitCommands;
		static std::vector<CommandBuffer*>	s_CommandBuffers;
	};
}
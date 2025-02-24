#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class CommandBuffer;


	class GENGINE_API GraphicsPresent
	{
	public:
		GraphicsPresent() {}
		virtual bool AquireImage() { return true; }
		virtual void Begin() {}
		virtual void End() {}

		static void AddWaitCommand(CommandBuffer* cmd)					{ s_WaitCommands.push_back(cmd); }
		static void SetWaitCommands(std::vector<CommandBuffer*> cmds)	{ s_WaitCommands = cmds; }
	protected:
		void OnWindowResize(const Vector2 size) { m_WindowResize = size; }

		static Scope<GraphicsPresent> Create();

		friend class Application;
	protected:
		Vector2								m_WindowResize = Vector2(0.0f, 0.0f);

		static std::vector<CommandBuffer*>	s_WaitCommands;
	};
}
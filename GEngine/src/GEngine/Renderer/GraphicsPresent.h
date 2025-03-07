#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Math/Math.h"
#include <set>

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
	protected:
		void OnWindowResize(const Vector2 size) { m_WindowResize = size; }

		static void AddWaitCommand(CommandBuffer* cmd)				{ s_WaitCommands.insert(cmd); }
		static void SetWaitCommands(std::set<CommandBuffer*> cmds)	{ s_WaitCommands = cmds; }
		static void ClearWaitCommands()								{ s_WaitCommands.clear(); }

		static Scope<GraphicsPresent> Create();

		friend class Application;
	protected:
		Vector2								m_WindowResize = Vector2(0.0f, 0.0f);

		static std::set<CommandBuffer*>	s_WaitCommands;
	};
}
#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API GraphicsPresent
	{
	public:
		GraphicsPresent() {}
		virtual void Begin() {}
		virtual void End() {}
		void OnWindowResize(const Vector2 size) { m_WindowSize = size; }
	public:
		static Scope<GraphicsPresent> Create();
	protected:
		Vector2		m_WindowSize = Vector2(0.0f, 0.0f);
	};
}
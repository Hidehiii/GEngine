#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		virtual void Init() = 0;
		virtual void Uninit() = 0;
		virtual void SwapBuffers() = 0;
	};
}

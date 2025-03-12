#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class CommandBuffer;

	class GENGINE_API GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		virtual void Init(const unsigned int width, const unsigned int height) = 0;
		virtual void Uninit() = 0;
		virtual void SwapBuffers() = 0;
		virtual void SetVSync(bool enable) = 0;
		virtual void SetRequiredExtensions(std::vector<const char*> extensions) = 0;
	public:
		bool m_WindowsResized = false;
	};
}

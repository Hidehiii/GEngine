#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API Graphics
	{
	public:
		virtual void Begin() {}
		virtual void End() {}
	public:
		static Ref<Graphics> Create();
	};
}
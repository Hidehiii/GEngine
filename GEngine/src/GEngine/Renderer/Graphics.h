#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class CommandBuffer;
	class GENGINE_API Graphics
	{
	public:
		static void RegisterSynchronousCommands(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second) {}

		static Ref<CommandBuffer>	GetGraphicsCommandBuffer() { return nullptr; }
		static Ref<CommandBuffer>	GetComputeCommandBuffer() { return nullptr; }
	};
}



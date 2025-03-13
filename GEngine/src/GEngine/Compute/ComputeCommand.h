#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Camera/EditorCamera.h"

namespace GEngine
{
	class CommandBuffer;
	class FrameBuffer;

	class GENGINE_API ComputeCommand
	{
	public:
		inline static Vector3 GetMaxComputeWorkGroupCount();
		inline static Vector3 GetMaxComputeWorkGroupSize();
		inline static uint32_t GetMaxComputeWorkGroupInvocations();

		inline static void BeginComputeCommand();
		inline static void EndComputeCommand();
		inline static void BeginSecondaryCommand();
		inline static void EndSecondaryCommand();
		inline static void Compute(const uint32_t x, const uint32_t y, const uint32_t z);

		inline static Ref<CommandBuffer>	BeginComputeCommand(Ref<FrameBuffer>& buffer, const Camera& camera);
		inline static Ref<CommandBuffer>	BeginComputeCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera);
		inline static void					EndComputeCommand(Ref<CommandBuffer> buffer);

		inline static void Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z);
	};
}



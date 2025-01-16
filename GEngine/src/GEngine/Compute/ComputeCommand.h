#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
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
	};
}



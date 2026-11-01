#pragma once
#include <GEngine.h>
#include <vector>

namespace GEngine
{
	class PipelineAssetChecks
	{
	public:
		virtual ~PipelineAssetChecks() = default;
		virtual void Execute(uint64_t frame, std::vector<CameraComponent>& cameras) = 0;
		virtual void Present(std::vector<CameraComponent>& cameras) = 0;
	};
	Ref<PipelineAssetChecks> CreatePipelineAssetChecks();
}

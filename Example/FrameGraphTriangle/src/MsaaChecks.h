#pragma once
#include <memory>
#include <cstdint>

namespace GEngine
{
	void RunMsaaChecks();
	class MsaaFrameChecks
	{
	public:
		virtual ~MsaaFrameChecks() = default;
		virtual void Run(uint64_t frame) = 0;
	};
	std::shared_ptr<MsaaFrameChecks> CreateMsaaFrameChecks();
}

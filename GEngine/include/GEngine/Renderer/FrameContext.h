#pragma once

#include "GEngine/Core/Core.h"

#include <cstdint>
#include <limits>

namespace GEngine
{
	class GENGINE_API FrameContext
	{
	public:
		static constexpr uint32_t InvalidImageIndex = std::numeric_limits<uint32_t>::max();

		void Begin(uint64_t frameNumber, uint8_t frameIndex)
		{
			m_FrameNumber = frameNumber;
			m_FrameIndex = frameIndex;
			m_PresentationImageIndex = InvalidImageIndex;
			m_SubmittedFenceValue = 0;
			m_Acquired = false;
			m_Submitted = false;
		}

		void MarkAcquired(uint32_t presentationImageIndex)
		{
			m_PresentationImageIndex = presentationImageIndex;
			m_Acquired = true;
		}

		void SetFrameIndex(uint8_t frameIndex)
		{
			m_FrameIndex = frameIndex;
		}

		void MarkSubmitted(uint64_t fenceValue = 0)
		{
			m_SubmittedFenceValue = fenceValue;
			m_Submitted = true;
		}

		uint64_t GetFrameNumber() const { return m_FrameNumber; }
		uint8_t GetFrameIndex() const { return m_FrameIndex; }
		uint32_t GetPresentationImageIndex() const { return m_PresentationImageIndex; }
		uint64_t GetSubmittedFenceValue() const { return m_SubmittedFenceValue; }
		bool IsAcquired() const { return m_Acquired; }
		bool IsSubmitted() const { return m_Submitted; }

	private:
		uint64_t m_FrameNumber = 0;
		uint64_t m_SubmittedFenceValue = 0;
		uint32_t m_PresentationImageIndex = InvalidImageIndex;
		uint8_t m_FrameIndex = 0;
		bool m_Acquired = false;
		bool m_Submitted = false;
	};
}

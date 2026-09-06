#pragma once
#include "GEngine/Graphics/GraphicsPresent.h"

namespace GEngine
{
	class GENGINE_API D3D12GraphicsPresent : public GraphicsPresent
	{
	public:
		D3D12GraphicsPresent();
		~D3D12GraphicsPresent() override;
		virtual bool AquireImage() override;
		virtual void Begin() override;
		virtual void End() override;
		virtual uint64_t Submit() override;
		virtual bool AcquireFrame(FrameContext& frameContext) override;
		virtual void BeginFrame(const FrameContext& frameContext) override;
		virtual Ref<GraphicsResource> GetPresentationResource() const override;

	private:
		std::vector<uint64_t>	m_FenceValues;
		std::vector<HANDLE>		m_FenceEvents;
		UINT					m_FrameIndex;
	};
}



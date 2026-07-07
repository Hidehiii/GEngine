#include "GEpch.h"
#include "D3D12GraphicsPresent.h"
#include "Platform/D3D12/D3D12Context.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	D3D12GraphicsPresent::D3D12GraphicsPresent()
	{
		s_CommandBuffers.resize(Graphics::GetFrameCount());
		m_FenceValues.resize(Graphics::GetFrameCount(), 0);
		m_FenceEvents.resize(Graphics::GetFrameCount(), nullptr);
		for (int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			s_CommandBuffers.at(i) = CreateRef<D3D12CommandBuffer>(COMMAND_BUFFER_TYPE_GRAPHICS);
			m_FenceEvents.at(i) = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		}

		m_FrameIndex = D3D12Context::Get()->GetSwapChain()->GetCurrentBackBufferIndex();
	}

	bool D3D12GraphicsPresent::AquireImage()
	{
		m_FrameIndex = D3D12Context::Get()->GetSwapChain()->GetCurrentBackBufferIndex();

		// wait until the previous frame is finished
		const uint64_t lastFrameFenceValue = m_FenceValues.at(m_FrameIndex);
		if (D3D12Context::Get()->GetFence(COMMAND_BUFFER_TYPE_GRAPHICS).first->GetCompletedValue() < lastFrameFenceValue)
		{
			D3D12Context::Get()->GetFence(COMMAND_BUFFER_TYPE_GRAPHICS).first->SetEventOnCompletion(lastFrameFenceValue, m_FenceEvents.at(m_FrameIndex));
			WaitForSingleObject(m_FenceEvents.at(m_FrameIndex), INFINITE);
		}
		return true;
	}

	void D3D12GraphicsPresent::Begin()
	{
		std::dynamic_pointer_cast<D3D12CommandBuffer>(s_CommandBuffers.at(m_FrameIndex))->BeginPresentRender(std::static_pointer_cast<FrameBuffer>(D3D12Context::Get()->GetRenderTarget(m_FrameIndex)));
	}

	void D3D12GraphicsPresent::End()
	{
		std::dynamic_pointer_cast<D3D12CommandBuffer>(s_CommandBuffers.at(m_FrameIndex))->EndPresentRender();
		if (D3D12Context::Get()->IsVSync())
		{
			UINT syncInterval = 1; // Enable VSync
			UINT presentFlags = 0;
			D3D12Context::Get()->GetSwapChain()->Present(syncInterval, presentFlags);
		}
		else
		{
			UINT syncInterval = 0; // Disable VSync
			UINT presentFlags = DXGI_PRESENT_ALLOW_TEARING;
			D3D12Context::Get()->GetSwapChain()->Present(syncInterval, presentFlags);
		}
		// signal and increment the fence value
		D3D12Context::Get()->IncreaseFenceValue(COMMAND_BUFFER_TYPE_GRAPHICS);
		auto f = D3D12Context::Get()->GetFence(COMMAND_BUFFER_TYPE_GRAPHICS);
		D3D12Context::Get()->GetGraphicsQueue()->Signal(f.first.Get(), f.second);
		m_FenceValues.at(m_FrameIndex) = f.second;
	}

}



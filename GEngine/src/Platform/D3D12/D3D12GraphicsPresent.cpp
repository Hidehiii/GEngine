#include "GEpch.h"
#include "Platform/D3D12/D3D12GraphicsPresent.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsResource.h"

namespace
{
	class D3D12PresentationResource final : public GEngine::GraphicsResource
	{
	public:
		explicit D3D12PresentationResource(ID3D12Resource* resource)
			: m_Resource(resource)
		{
			GE_CORE_ASSERT(m_Resource != nullptr, "D3D12 presentation resource is invalid.");
		}

		GEngine::GraphicsResourceType GetResourceType() const override { return GEngine::GraphicsResourceType::Texture; }

	private:
		void* GetNativeResource() const override { return m_Resource; }
		ID3D12Resource* m_Resource = nullptr;
	};
}

namespace GEngine
{
	D3D12GraphicsPresent::D3D12GraphicsPresent()
	{
		m_CommandBuffers.resize(Graphics::GetFrameCount());
		m_FenceValues.resize(Graphics::GetFrameCount(), 0);
		m_FenceEvents.resize(Graphics::GetFrameCount(), nullptr);
		for (int i = 0; i < Graphics::GetFrameCount(); i++)
		{
			m_CommandBuffers.at(i) = CreateRef<D3D12CommandBuffer>(COMMAND_BUFFER_TYPE_GRAPHICS);
			m_FenceEvents.at(i) = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		}

		m_FrameIndex = D3D12Context::Get()->GetSwapChain()->GetCurrentBackBufferIndex();
	}

	D3D12GraphicsPresent::~D3D12GraphicsPresent()
	{
		// Command allocators and lists are released by the base class immediately
		// after this destructor.  Wait for every submitted back-buffer slot before
		// releasing them and close the per-slot Win32 events we own.
		auto fence = D3D12Context::Get()->GetFence(COMMAND_BUFFER_TYPE_GRAPHICS).first;
		for (size_t index = 0; index < m_FenceValues.size(); ++index)
		{
			const auto value = m_FenceValues[index];
			if (value != 0 && fence->GetCompletedValue() < value)
			{
				D3D12_THROW_IF_FAILED(fence->SetEventOnCompletion(value, m_FenceEvents.at(index)));
				WaitForSingleObject(m_FenceEvents.at(index), INFINITE);
			}
			if (m_FenceEvents[index] != nullptr)
				CloseHandle(m_FenceEvents[index]);
		}
		m_FenceEvents.clear();
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

	bool D3D12GraphicsPresent::AcquireFrame(FrameContext& frameContext)
	{
		if (!AquireImage())
			return false;

		frameContext.SetFrameIndex(static_cast<uint8_t>(m_FrameIndex));
		frameContext.MarkAcquired(m_FrameIndex);
		return true;
	}

	void D3D12GraphicsPresent::Begin()
	{
		std::dynamic_pointer_cast<D3D12CommandBuffer>(m_CommandBuffers.at(m_FrameIndex))->BeginPresentRender(std::static_pointer_cast<FrameBuffer>(D3D12Context::Get()->GetRenderTarget(m_FrameIndex)));
	}

	void D3D12GraphicsPresent::BeginFrame(const FrameContext& frameContext)
	{
		GE_CORE_ASSERT(frameContext.GetPresentationImageIndex() == m_FrameIndex, "D3D12 presentation image changed between acquire and begin.");
		auto commandBuffer = std::dynamic_pointer_cast<D3D12CommandBuffer>(m_CommandBuffers.at(m_FrameIndex));
		// AcquireFrame waited for this back-buffer slot's fence.  Resetting the
		// allocator here releases command memory accumulated by its previous use
		// before the list records another frame.
		D3D12_THROW_IF_FAILED(commandBuffer->GetCommandAllocator()->Reset());
		D3D12_THROW_IF_FAILED(commandBuffer->GetCommandList()->Reset(commandBuffer->GetCommandAllocator().Get(), nullptr));
	}

	void D3D12GraphicsPresent::End()
	{
		std::dynamic_pointer_cast<D3D12CommandBuffer>(m_CommandBuffers.at(m_FrameIndex))->EndPresentRender();
	}

	uint64_t D3D12GraphicsPresent::Submit()
	{
		Graphics::SubmitCommandBuffer(m_CommandBuffers.at(m_FrameIndex));
		if (D3D12Context::Get()->IsVSync())
		{
			UINT syncInterval = 1; // Enable VSync
			UINT presentFlags = 0;
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetSwapChain()->Present(syncInterval, presentFlags));
		}
		else
		{
			UINT syncInterval = 0; // Disable VSync
			UINT presentFlags = DXGI_PRESENT_ALLOW_TEARING;
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetSwapChain()->Present(syncInterval, presentFlags));
		}
		// signal and increment the fence value
		D3D12Context::Get()->IncreaseFenceValue(COMMAND_BUFFER_TYPE_GRAPHICS);
		auto f = D3D12Context::Get()->GetFence(COMMAND_BUFFER_TYPE_GRAPHICS);
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetGraphicsQueue()->Signal(f.first.Get(), f.second));
		m_FenceValues.at(m_FrameIndex) = f.second;
		return f.second;
	}

	Ref<GraphicsResource> D3D12GraphicsPresent::GetPresentationResource() const
	{
		return CreateRef<D3D12PresentationResource>(
			D3D12Context::Get()->GetRenderTarget(m_FrameIndex)->GetNativeRenderTarget(0));
	}

}



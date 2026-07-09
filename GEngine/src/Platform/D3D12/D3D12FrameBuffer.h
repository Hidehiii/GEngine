#pragma once
#include "GEngine/Graphics/FrameBuffer.h"
#include "Platform/D3D12/D3D12RenderPass.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12DescriptorHeap.h"
#include <directx/d3dx12.h>

namespace GEngine
{
	struct FrameBufferSpecificationForD3D12
	{
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		RenderTargets;
		uint32_t												Width;
		uint32_t												Height;
	};


	class GENGINE_API D3D12FrameBuffer : public FrameBuffer
	{
	public:
		D3D12FrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height);
		D3D12FrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		D3D12FrameBuffer(const Ref<D3D12RenderPass>& renderpass, const FrameBufferSpecificationForD3D12& spec, const RenderPassSpecificationForD3D12& renderpassSpec);
		virtual ~D3D12FrameBuffer() override;

		virtual void Begin(CommandBuffer* cmdBuffer) override;
		virtual void End(CommandBuffer* cmdBuffer) override;

		virtual void SetRenderPassOperation(const RenderPassOperation& op) override;

		virtual int								GetRenderTargetCount() override { return m_RenderTargetResources.size(); }
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual Ref<Texture2D>					GetRenderTarget(int index) override;
		virtual Ref<Texture2D>					GetDepthStencil() override;
		virtual Ref<RenderPass>					GetRenderPass() override { return std::static_pointer_cast<RenderPass>(m_RenderPass); }
	protected:
		// for present
		void BeginPresentRender(CommandBuffer* cmdBuffer) override;
		void EndPresentRender(CommandBuffer* cmdBuffer) override;

	private:
		void CreateResources();
	private:
		D3D12DescriptorHeap::D3D12DescriptorAllocationInfo		m_RtvHeap;
		D3D12DescriptorHeap::D3D12DescriptorAllocationInfo		m_MultiSampleRtvHeap;
		D3D12DescriptorHeap::D3D12DescriptorAllocationInfo		m_DsvHeap;
		D3D12DescriptorHeap::D3D12DescriptorAllocationInfo		m_MultiSampleDsvHeap;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		m_RenderTargetResources;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		m_MultiSampleRenderTargetResources;
		Microsoft::WRL::ComPtr<ID3D12Resource>					m_DepthStencilResource = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource>					m_MultiSampleDepthStencilResource = nullptr;
		Ref<D3D12RenderPass>									m_RenderPass;
		std::vector<Ref<D3D12Texture2D>>						m_RenderTargets;
		Ref<D3D12Texture2D>										m_DepthStencil = nullptr;

		friend class D3D12CommandBuffer;
	};

}


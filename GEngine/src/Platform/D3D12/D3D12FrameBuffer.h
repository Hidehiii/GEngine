#pragma once
#include "GEngine/Graphics/FrameBuffer.h"
#include "D3D12RenderPass.h"
#include "D3D12Texture2D.h"
#include <directx/d3dx12.h>

namespace GEngine
{
	struct FrameBufferSpecificationForD3D12
	{
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		ColorRTs;
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

		virtual int								GetColorRTCount() override { return m_ColorRenderTargets.size(); }
		virtual int								GetRTCount() override { return m_DepthStencilRenderTarget == nullptr ? m_ColorRenderTargets.size() : m_ColorRenderTargets.size() + 1; };
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual Ref<Texture2D>					GetColorRT(int index) override;
		virtual Ref<Texture2D>					GetDepthStencilRT() override;
		virtual Ref<RenderPass>					GetRenderPass() override { return std::static_pointer_cast<RenderPass>(m_RenderPass); }

		// for present
		void BeginPresentRender(CommandBuffer* cmdBuffer);
		void EndPresentRender(CommandBuffer* cmdBuffer);

	private:
		void CreateResources();
	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_MultiSampleRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_DsvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_MultiSampleDsvHeap;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		m_ColorRenderTargets;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		m_MultiSampleColorRenderTargets;
		Microsoft::WRL::ComPtr<ID3D12Resource>					m_DepthStencilRenderTarget = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource>					m_MultiSampleDepthStencilRenderTarget = nullptr;
		Ref<D3D12RenderPass>									m_RenderPass;
		std::vector<Ref<D3D12Texture2D>>						m_ColorRTs;
		Ref<D3D12Texture2D>										m_DepthStencilRT = nullptr;
	};

}


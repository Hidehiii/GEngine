#pragma once
#include "GEngine/Graphics/FrameBuffer.h"
#include "D3D12RenderPass.h"
#include <directx/d3dx12.h>

namespace GEngine
{
	class GENGINE_API D3D12FrameBuffer : public FrameBuffer
	{
	public:
		D3D12FrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height);
		D3D12FrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		virtual ~D3D12FrameBuffer() override;

		virtual void Begin(CommandBuffer* cmdBuffer) override;
		virtual void End(CommandBuffer* cmdBuffer) override;

		virtual void SetRenderPassOperation(const RenderPassOperation& op) override;

		virtual int								GetColorRTCount() override { return m_ColorAttachments.size(); }
		virtual int								GetRTCount() override { return m_DepthStencilAttachment == 0 ? m_ColorAttachments.size() : m_ColorAttachments.size() + 1; };
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual Ref<Texture2D>					GetColorRT(int index) override;
		virtual Ref<Texture2D>					GetDepthStencilRT() override;
		virtual Ref<RenderPass>					GetRenderPass() override { return std::static_pointer_cast<RenderPass>(m_RenderPass); }

	private:
		void CreateResources();
	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>			m_DsvHeap;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>		m_ColorRenderTargets;
		Microsoft::WRL::ComPtr<ID3D12Resource>					m_DepthStencilRenderTarget = nullptr;
		Ref<D3D12RenderPass>									m_RenderPass;
	};

}


#include "GEpch.h"
#include "D3D12FrameBuffer.h"
#include "D3D12Utils.h"
#include "D3D12Context.h"
#include "D3D12CommandBuffer.h"

namespace GEngine
{
	D3D12FrameBuffer::D3D12FrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
	{
		m_Specification.ColorRTs			= renderPass->GetSpecification().ColorRTs;
		m_Specification.DepthStencilRT		= renderPass->GetSpecification().DepthStencilRT;
		m_Specification.Samples				= renderPass->GetSpecification().Samples;
		m_Specification.Width				= width;
		m_Specification.Height				= height;

		m_RenderPass = std::dynamic_pointer_cast<D3D12RenderPass>(renderPass);

		D3D12_DESCRIPTOR_HEAP_DESC	heapDesc = {};
		heapDesc.NumDescriptors		= m_Specification.ColorRTs.size();
		heapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		// TODO : shader visiable?
		heapDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RtvHeap)));

		if (m_Specification.DepthStencilRT.TextureFormat != FrameBufferTextureFormat::None)
		{
			heapDesc.NumDescriptors = 1;
			heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeap)));
		}
		

		CreateResources();
	}
	void D3D12FrameBuffer::Begin(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer>(cmdBuffer)->GetCommandList();

		for (int i = 0; i < m_ColorRTs.size(); i++)
		{
			m_ColorRTs.at(i)->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		if (m_DepthStencilRT)
		{
			m_DepthStencilRT->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}
	void D3D12FrameBuffer::End(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer>(cmdBuffer)->GetCommandList();

		for (int i = 0; i < m_ColorRTs.size(); i++)
		{
			m_ColorRTs.at(i)->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}
		if (m_DepthStencilRT)
		{
			m_DepthStencilRT->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}
	}
	Ref<Texture2D> D3D12FrameBuffer::GetColorRT(int index)
	{
		return m_ColorRTs.at(index);
	}
	Ref<Texture2D> D3D12FrameBuffer::GetDepthStencilRT()
	{
		return m_DepthStencilRT;
	}
	void D3D12FrameBuffer::CreateResources()
	{
		m_ColorRenderTargets.resize(m_Specification.ColorRTs.size());
		m_ColorRTs.resize(m_Specification.ColorRTs.size());

		for (UINT i = 0; i < m_Specification.ColorRTs.size(); i++)
		{
			const FLOAT v[4] = { 0, 0, 0, 0 };
			const CD3DX12_CLEAR_VALUE clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat), v);

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
				Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat),
				m_Specification.Width,
				m_Specification.Height,
				1, // This resource has only one texture.
				1  // Use a single mipmap level.
			);

			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
				&desc,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				&clearValue,
				IID_PPV_ARGS(m_ColorRenderTargets[i].GetAddressOf())));

			D3D12_RENDER_TARGET_VIEW_DESC	rtvDesc = {};
			rtvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat);
			rtvDesc.ViewDimension			= D3D12_RTV_DIMENSION_TEXTURE2D;

			const auto cpuHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(cpuHandle, static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorSize());
			D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_ColorRenderTargets[i].Get(), &rtvDesc, rtvDescriptor);

			m_ColorRTs.at(i) = CreateRef<D3D12Texture2D>(m_ColorRenderTargets[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		if (m_Specification.DepthStencilRT.TextureFormat != FrameBufferTextureFormat::None)
		{
			const CD3DX12_CLEAR_VALUE clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat), 1, 0);

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
				Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat),
				m_Specification.Width,
				m_Specification.Height,
				1, // This resource has only one texture.
				1  // Use a single mipmap level.
			);

			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(m_DepthStencilRenderTarget.GetAddressOf())));
			D3D12_DEPTH_STENCIL_VIEW_DESC	dsvDesc = {};
			dsvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat);
			dsvDesc.ViewDimension			= D3D12_DSV_DIMENSION_TEXTURE2D;

			const auto cpuHandle = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
			D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_DepthStencilRenderTarget.Get(), &dsvDesc, cpuHandle);

			m_DepthStencilRT = CreateRef<D3D12Texture2D>(m_DepthStencilRenderTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}
}
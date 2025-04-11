#include "GEpch.h"
#include "D3D12FrameBuffer.h"
#include "D3D12Utils.h"
#include "D3D12Context.h"
#include "D3D12CommandBuffer.h"
#include "GEngine/Graphics/Graphics.h"

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
		if (m_Specification.Samples > 1)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_MultiSampleRtvHeap)));
		}

		if (m_Specification.DepthStencilRT.TextureFormat != FrameBufferTextureFormat::None)
		{
			heapDesc.NumDescriptors = 1;
			heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeap)));

			if (m_Specification.Samples > 1)
			{
				D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_MultiSampleDsvHeap)));
			}
		}
		

		CreateResources();
	}
	D3D12FrameBuffer::D3D12FrameBuffer(const Ref<D3D12RenderPass>& renderpass, const FrameBufferSpecificationForD3D12& spec, const RenderPassSpecificationForD3D12& renderpassSpec)
	{
		m_Specification.Width	= spec.Width;
		m_Specification.Height	= spec.Height;
		m_RenderPass			= std::dynamic_pointer_cast<D3D12RenderPass>(renderpass);
		m_Specification.Samples = renderpassSpec.Samples;

		m_ColorRenderTargets	= spec.ColorRTs;

		D3D12_DESCRIPTOR_HEAP_DESC	heapDesc = {};
		heapDesc.NumDescriptors = spec.ColorRTs.size();
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		// TODO : shader visiable?
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RtvHeap)));

		for (int i = 0; i < spec.ColorRTs.size(); i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC	rtvDesc = {};
			rtvDesc.Format					= renderpassSpec.BackBufferFormat[i];
			rtvDesc.ViewDimension			= D3D12_RTV_DIMENSION_TEXTURE2D;

			const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
				m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
				static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorSize());
			D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_ColorRenderTargets[i].Get(), &rtvDesc, rtvDescriptor);
		}

		if (renderpassSpec.EnableDepthStencil)
		{
			m_Specification.DepthStencilRT.TextureFormat = FrameBufferTextureFormat::DEPTH24STENCIL8;

			heapDesc.NumDescriptors = 1;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeap)));

			const CD3DX12_CLEAR_VALUE		clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat), 1, 0);
			D3D12_DEPTH_STENCIL_VIEW_DESC	dsvDesc = {};
			dsvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat);
			dsvDesc.ViewDimension			= D3D12_DSV_DIMENSION_TEXTURE2D;

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
				Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat),
				m_Specification.Width,
				m_Specification.Height,
				1, // This resource has only one texture.
				1,  // Use a single mipmap level.
				m_Specification.Samples
			);

			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(m_DepthStencilRenderTarget.GetAddressOf())));


			D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_DepthStencilRenderTarget.Get(), &dsvDesc, m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
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

		auto rtvDescriptor = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		auto dsvDescriptor = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

		if (m_Specification.Samples > 1)
		{
			rtvDescriptor = m_MultiSampleRtvHeap->GetCPUDescriptorHandleForHeapStart();
			dsvDescriptor = m_MultiSampleDsvHeap->GetCPUDescriptorHandleForHeapStart();	
		}

		cmd->OMSetRenderTargets(1, &rtvDescriptor, TRUE, &dsvDescriptor);
		if (m_RenderPass->GetSpecification().Operation.ColorBegin == RenderPassBeginOperation::Clear)
		{
			cmd->ClearRenderTargetView(rtvDescriptor, Math::ValuePtr(D3D12Context::Get()->GetClearColor()), 0, nullptr);
		}
		if (m_RenderPass->GetSpecification().Operation.DepthStencilBegin == RenderPassBeginOperation::Clear)
		{
			cmd->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Graphics::IsReverseDepth() ? 0 : 1.0f, 0, 0, nullptr);
		}

		D3D12_VIEWPORT							viewPort = {};
		viewPort.TopLeftX = viewPort.TopLeftY	= 0;
		viewPort.Width							= static_cast<float>(m_Specification.Width);
		viewPort.Height							= static_cast<float>(m_Specification.Height);
		viewPort.MinDepth						= Graphics::IsReverseDepth() ? D3D12_MAX_DEPTH : D3D12_MIN_DEPTH;
		viewPort.MaxDepth						= Graphics::IsReverseDepth() ? D3D12_MIN_DEPTH : D3D12_MAX_DEPTH;

		cmd->RSSetViewports(1, &viewPort);

		D3D12_RECT				rect = {};
		rect.left = rect.top	= 0;
		rect.right				= static_cast<LONG>(m_Specification.Width);
		rect.bottom				= static_cast<LONG>(m_Specification.Height);

		cmd->RSSetScissorRects(1, &rect);
	}
	void D3D12FrameBuffer::End(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer>(cmdBuffer)->GetCommandList();

		// Resolve the MSAA render target.
		if (m_Specification.Samples > 1)
		{
			std::vector< D3D12_RESOURCE_BARRIER>	barriers;
			for (int i = 0; i < m_ColorRTs.size(); i++)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleColorRenderTargets[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));
				m_ColorRTs.at(i)->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			}
			if (m_DepthStencilRT)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleDepthStencilRenderTarget.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));
				m_DepthStencilRT->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			}
			cmd->ResourceBarrier(barriers.size(), barriers.data());
			for (int i = 0; i < m_ColorRTs.size(); i++)
			{
				cmd->ResolveSubresource(m_ColorRenderTargets[i].Get(), 0, m_MultiSampleColorRenderTargets[i].Get(), 0, Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat));
			}
			if (m_DepthStencilRT)
			{
				cmd->ResolveSubresource(m_DepthStencilRenderTarget.Get(), 0, m_MultiSampleDepthStencilRenderTarget.Get(), 0, Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat));
			}
			barriers.clear();
			for (int i = 0; i < m_ColorRTs.size(); i++)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleColorRenderTargets[i].Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
				
			}
			if (m_DepthStencilRT)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleDepthStencilRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
			}
		}
		

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
	void D3D12FrameBuffer::BeginPresentRender(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer>(cmdBuffer)->GetCommandList();

		std::vector< D3D12_RESOURCE_BARRIER>	barriers;
		for (int i = 0; i < m_ColorRTs.size(); i++)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_ColorRenderTargets[i].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
		cmd->ResourceBarrier(barriers.size(), barriers.data());

		auto rtvDescriptor = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		auto dsvDescriptor = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();

		cmd->OMSetRenderTargets(1, &rtvDescriptor, TRUE, &dsvDescriptor);
		if (m_RenderPass->GetSpecification().Operation.ColorBegin == RenderPassBeginOperation::Clear)
		{
			cmd->ClearRenderTargetView(rtvDescriptor, Math::ValuePtr(D3D12Context::Get()->GetClearColor()), 0, nullptr);
		}
		if (m_RenderPass->GetSpecification().Operation.DepthStencilBegin == RenderPassBeginOperation::Clear)
		{
			cmd->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Graphics::IsReverseDepth() ? 0 : 1.0f, 0, 0, nullptr);
		}

		D3D12_VIEWPORT							viewPort = {};
		viewPort.TopLeftX						= viewPort.TopLeftY = 0;
		viewPort.Width							= static_cast<float>(m_Specification.Width);
		viewPort.Height							= static_cast<float>(m_Specification.Height);
		viewPort.MinDepth						= Graphics::IsReverseDepth() ? D3D12_MAX_DEPTH : D3D12_MIN_DEPTH;
		viewPort.MaxDepth						= Graphics::IsReverseDepth() ? D3D12_MIN_DEPTH : D3D12_MAX_DEPTH;

		cmd->RSSetViewports(1, &viewPort);

		D3D12_RECT				rect = {};
		rect.left				= rect.top = 0;
		rect.right				= static_cast<LONG>(m_Specification.Width);
		rect.bottom				= static_cast<LONG>(m_Specification.Height);

		cmd->RSSetScissorRects(1, &rect);
	}
	void D3D12FrameBuffer::EndPresentRender(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer>(cmdBuffer)->GetCommandList();

		std::vector< D3D12_RESOURCE_BARRIER>	barriers;
		for (int i = 0; i < m_ColorRTs.size(); i++)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_ColorRenderTargets[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		}
		cmd->ResourceBarrier(barriers.size(), barriers.data());
	}
	void D3D12FrameBuffer::CreateResources()
	{
		m_ColorRenderTargets.resize(m_Specification.ColorRTs.size());
		m_MultiSampleColorRenderTargets.resize(m_Specification.ColorRTs.size());
		m_ColorRTs.resize(m_Specification.ColorRTs.size());

		for (UINT i = 0; i < m_Specification.ColorRTs.size(); i++)
		{
			const FLOAT v[4] = { 0, 0, 0, 0 };
			const CD3DX12_CLEAR_VALUE clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat), v);

			D3D12_RENDER_TARGET_VIEW_DESC	rtvDesc = {};
			rtvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat);
			rtvDesc.ViewDimension			= D3D12_RTV_DIMENSION_TEXTURE2D;

			if (m_Specification.Samples > 1)
			{
				D3D12_RESOURCE_DESC multiDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.ColorRTs[i].TextureFormat),
					m_Specification.Width,
					m_Specification.Height,
					1, // This resource has only one texture.
					1,  // Use a single mipmap level.
					m_Specification.Samples
				);

				multiDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
					&multiDesc,
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&clearValue,
					IID_PPV_ARGS(m_MultiSampleColorRenderTargets[i].GetAddressOf())));

				const CD3DX12_CPU_DESCRIPTOR_HANDLE multiSampleRtvDescriptor(m_MultiSampleRtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorSize());
				D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_MultiSampleColorRenderTargets[i].Get(), &rtvDesc, multiSampleRtvDescriptor);
			}

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

			const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorSize());
			D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_ColorRenderTargets[i].Get(), &rtvDesc, rtvDescriptor);

			m_ColorRTs.at(i) = CreateRef<D3D12Texture2D>(m_ColorRenderTargets[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		if (m_Specification.DepthStencilRT.TextureFormat != FrameBufferTextureFormat::None)
		{
			const CD3DX12_CLEAR_VALUE clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat), 1, 0);
			D3D12_DEPTH_STENCIL_VIEW_DESC	dsvDesc = {};
			dsvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat);
			dsvDesc.ViewDimension			= D3D12_DSV_DIMENSION_TEXTURE2D;

			if (m_Specification.Samples > 1)
			{
				D3D12_RESOURCE_DESC multiDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencilRT.TextureFormat),
					m_Specification.Width,
					m_Specification.Height,
					1, // This resource has only one texture.
					1,  // Use a single mipmap level.
					m_Specification.Samples
				);

				multiDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


				D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
					&multiDesc,
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&clearValue,
					IID_PPV_ARGS(m_MultiSampleDepthStencilRenderTarget.GetAddressOf())));

				D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_MultiSampleDepthStencilRenderTarget.Get(), &dsvDesc, m_MultiSampleDsvHeap->GetCPUDescriptorHandleForHeapStart());
			}

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
			

			D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_DepthStencilRenderTarget.Get(), &dsvDesc, m_DsvHeap->GetCPUDescriptorHandleForHeapStart());

			m_DepthStencilRT = CreateRef<D3D12Texture2D>(m_DepthStencilRenderTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}
}
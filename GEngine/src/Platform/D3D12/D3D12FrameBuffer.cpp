#include "GEpch.h"
#include "Platform/D3D12/D3D12FrameBuffer.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12CommandBuffer.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	D3D12FrameBuffer::D3D12FrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
	{
		m_Specification.RenderTargets		= renderPass->GetSpecification().RenderTargets;
		m_Specification.DepthStencil		= renderPass->GetSpecification().DepthStencil;
		m_Specification.Samples				= renderPass->GetSpecification().Samples;
		m_Specification.Width				= width;
		m_Specification.Height				= height;

		m_RenderPass = std::dynamic_pointer_cast<D3D12RenderPass>(renderPass);

		try
		{
		m_RtvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_Specification.RenderTargets.size());

		if (m_Specification.Samples > 1)
		{
			m_MultiSampleRtvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_Specification.RenderTargets.size());
		}

		if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
		{
			m_DsvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

			if (m_Specification.Samples > 1)
			{
				m_MultiSampleDsvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
			}
		}
		
		CreateResources();
		}
		catch (...) { ReleaseDescriptors(); throw; }
	}
	D3D12FrameBuffer::D3D12FrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
		: D3D12FrameBuffer(buffer->GetRenderPass(), width, height)
	{
	}
	D3D12FrameBuffer::D3D12FrameBuffer(const Ref<D3D12RenderPass>& renderpass, const FrameBufferSpecificationForD3D12& spec, const RenderPassSpecificationForD3D12& renderpassSpec)
	{
		// we have not assume multi sample, so we just use 1 sample for now
		m_Specification.Width	= spec.Width;
		m_Specification.Height	= spec.Height;
		m_RenderPass			= std::dynamic_pointer_cast<D3D12RenderPass>(renderpass);
		m_Specification.Samples = renderpassSpec.Samples;
		if (m_Specification.Samples > 1)
			throw std::invalid_argument("Swap-chain MSAA is unsupported; resolve an offscreen target before presentation.");
		for (const auto format : renderpassSpec.BackBufferFormat)
			m_Specification.RenderTargets.push_back(Utils::DXGIFormatToFrameBufferTextureFormat(format));
		m_Specification.DepthStencil = renderpassSpec.EnableDepthStencil
			? FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8
			: FRAME_BUFFER_TEXTURE_FORMAT_NONE;

		m_RenderTargetResources = spec.RenderTargets;

		m_RtvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_RenderTargetResources.size());

		if(m_Specification.Samples > 1)
		{
			m_MultiSampleRtvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_RenderTargetResources.size());

			m_MultiSampleRenderTargetResources.resize(m_RenderTargetResources.size());
		}

		for (int i = 0; i < spec.RenderTargets.size(); i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC	rtvDesc = {};
			rtvDesc.Format					= renderpassSpec.BackBufferFormat[i];
			rtvDesc.ViewDimension			= D3D12_RTV_DIMENSION_TEXTURE2D;

			for(int frame = 0; frame < Graphics::GetFrameCount(); frame++)
			{
				const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_RtvHeap.CpuHandles.at(frame),static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorIncrementSize());
				D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_RenderTargetResources[i].Get(), &rtvDesc, rtvDescriptor);
			}
		}

		if (renderpassSpec.EnableDepthStencil)
		{
			m_Specification.DepthStencil = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;

			m_DsvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

			if (m_Specification.Samples > 1)
			{
				m_MultiSampleDsvHeap = D3D12Context::Get()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
			}

			const CD3DX12_CLEAR_VALUE		clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencil), Graphics::IsReverseDepth() ? 0.0f : 1.0f, 0);
			D3D12_DEPTH_STENCIL_VIEW_DESC	dsvDesc = {};
			dsvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencil);
			dsvDesc.ViewDimension			= D3D12_DSV_DIMENSION_TEXTURE2D;

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
				Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencil),
				m_Specification.Width,
				m_Specification.Height,
				1, // This resource has only one texture.
				1,  // Use a single mipmap level.
				m_Specification.Samples
			);

			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(m_DepthStencilResource.GetAddressOf())));

			for(int frame = 0; frame < Graphics::GetFrameCount(); frame++)
			{
				const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor(m_DsvHeap.CpuHandles.at(frame), 0, D3D12Context::Get()->GetDsvDescriptorIncrementSize());
				D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_DepthStencilResource.Get(), &dsvDesc, dsvDescriptor);
			}
		}
	}
	D3D12FrameBuffer::~D3D12FrameBuffer()
	{
		ReleaseDescriptors();
	}
	void D3D12FrameBuffer::ReleaseDescriptors()
	{
		for (auto* allocation : { &m_RtvHeap, &m_MultiSampleRtvHeap, &m_DsvHeap, &m_MultiSampleDsvHeap })
		{
			if (allocation->IsValid()) D3D12Context::Get()->FreeDescriptor(*allocation);
			*allocation = {};
		}
	}
	void D3D12FrameBuffer::Begin(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer*>(cmdBuffer)->GetCommandList();

		for (int i = 0; i < m_RenderTargets.size(); i++)
		{
			m_RenderTargets.at(i)->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		if (m_DepthStencil)
		{
			m_DepthStencil->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		auto rtvDescriptor = m_RtvHeap.CpuHandles.at(Graphics::GetFrame());
		const bool hasDepth = m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor{};
		if (hasDepth) dsvDescriptor = m_DsvHeap.CpuHandles.at(Graphics::GetFrame());

		if (m_Specification.Samples > 1)
		{
			rtvDescriptor = m_MultiSampleRtvHeap.CpuHandles.at(Graphics::GetFrame());
			if (hasDepth) dsvDescriptor = m_MultiSampleDsvHeap.CpuHandles.at(Graphics::GetFrame());
		}

		cmd->OMSetRenderTargets(static_cast<UINT>(m_Specification.RenderTargets.size()), &rtvDescriptor, TRUE, hasDepth ? &dsvDescriptor : nullptr);
		if (m_RenderPass->GetSpecification().Operation.ColorBegin == RENDER_PASS_BEGINE_OP_CLEAR)
		{
			for (uint32_t index = 0; index < m_Specification.RenderTargets.size(); ++index)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvDescriptor, index, D3D12Context::Get()->GetRtvDescriptorIncrementSize());
				cmd->ClearRenderTargetView(handle, Math::ValuePtr(D3D12Context::Get()->GetClearColor()), 0, nullptr);
			}
		}
		if (hasDepth && m_RenderPass->GetSpecification().Operation.DepthStencilBegin == RENDER_PASS_BEGINE_OP_CLEAR)
		{
			const auto flags = m_Specification.DepthStencil == FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8 ?
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL : D3D12_CLEAR_FLAG_DEPTH;
			cmd->ClearDepthStencilView(dsvDescriptor, flags, Graphics::IsReverseDepth() ? 0 : 1.0f, 0, 0, nullptr);
		}

		D3D12_VIEWPORT							viewPort = {};
		viewPort.TopLeftX = viewPort.TopLeftY	= 0;
		viewPort.Width							= static_cast<float>(m_Specification.Width);
		viewPort.Height							= static_cast<float>(m_Specification.Height);
		viewPort.MinDepth = D3D12_MIN_DEPTH;
		viewPort.MaxDepth = D3D12_MAX_DEPTH;

		cmd->RSSetViewports(1, &viewPort);

		D3D12_RECT				rect = {};
		rect.left = rect.top	= 0;
		rect.right				= static_cast<LONG>(m_Specification.Width);
		rect.bottom				= static_cast<LONG>(m_Specification.Height);

		cmd->RSSetScissorRects(1, &rect);
	}
	void D3D12FrameBuffer::End(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer*>(cmdBuffer)->GetCommandList();

		// Resolve the MSAA render target.
		if (m_Specification.Samples > 1)
		{
			if (m_DepthStencil)
			{
				Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> resolve;
				D3D12_THROW_IF_FAILED(cmd.As(&resolve));
				D3D12_RESOURCE_BARRIER source = CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleDepthStencilResource.Get(),
					D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
				cmd->ResourceBarrier(1, &source);
				m_DepthStencil->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_RESOLVE_DEST);
				const auto requested = m_RenderPass->GetSpecification().DepthResolve;
				const auto mode = requested == DepthResolveMode::Maximum ||
					(requested == DepthResolveMode::BackendDefault && Graphics::IsReverseDepth()) ?
					D3D12_RESOLVE_MODE_MAX : D3D12_RESOLVE_MODE_MIN;
				const bool stencil = m_Specification.DepthStencil == FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
				resolve->ResolveSubresourceRegion(m_DepthStencilResource.Get(), 0, 0, 0,
					m_MultiSampleDepthStencilResource.Get(), 0, nullptr,
					stencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT, mode);
				if (stencil)
					resolve->ResolveSubresourceRegion(m_DepthStencilResource.Get(), 1, 0, 0,
						m_MultiSampleDepthStencilResource.Get(), 1, nullptr, DXGI_FORMAT_X24_TYPELESS_G8_UINT,
						D3D12_RESOLVE_MODE_MIN);
				std::swap(source.Transition.StateBefore, source.Transition.StateAfter);
				cmd->ResourceBarrier(1, &source);
			}
			std::vector< D3D12_RESOURCE_BARRIER>	barriers;
			// transition the MSAA render target to resolve source and the resolve destination to resolve destination
			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleRenderTargetResources[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));
				m_RenderTargets.at(i)->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			}
			cmd->ResourceBarrier(barriers.size(), barriers.data());
			barriers.clear();
			// resolve the MSAA render target to the resolve destination
			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				cmd->ResolveSubresource(m_RenderTargetResources[i].Get(), 0, m_MultiSampleRenderTargetResources[i].Get(), 0, Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.RenderTargets[i]));
			}
			// transition the MSAA render target back to render target and the resolve destination back to shader resource
			for (int i = 0; i < m_RenderTargets.size(); i++)
			{
				barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_MultiSampleRenderTargetResources[i].Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
				
			}
			cmd->ResourceBarrier(barriers.size(), barriers.data());
		}
		

		for (int i = 0; i < m_RenderTargets.size(); i++)
		{
			m_RenderTargets.at(i)->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}
		if (m_DepthStencil)
		{
			m_DepthStencil->TransitionResourceState(cmd, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}
	}
	void D3D12FrameBuffer::SetRenderPassOperation(const RenderPassOperation& op)
	{
		auto spec		= m_RenderPass->GetSpecification();
		spec.Operation	= op;
		m_RenderPass	= std::dynamic_pointer_cast<D3D12RenderPass>(RenderPass::Create(spec));
	}
	Ref<Texture2D> D3D12FrameBuffer::GetRenderTarget(uint32_t index)
	{
		return Ref<Texture2D>(shared_from_this(), m_RenderTargets.at(index).get());
	}
	Ref<Texture2D> D3D12FrameBuffer::GetDepthStencil()
	{
		return Ref<Texture2D>(shared_from_this(), m_DepthStencil.get());
	}
	void D3D12FrameBuffer::BeginPresentRender(CommandBuffer* cmdBuffer)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd = static_cast<D3D12CommandBuffer*>(cmdBuffer)->GetCommandList();

		auto rtvDescriptor = m_RtvHeap.CpuHandles.at(Graphics::GetFrame());
		auto dsvDescriptor = m_DsvHeap.CpuHandles.at(Graphics::GetFrame());

		if(m_Specification.Samples > 1)
		{
			rtvDescriptor = m_MultiSampleRtvHeap.CpuHandles.at(Graphics::GetFrame());
			dsvDescriptor = m_MultiSampleDsvHeap.CpuHandles.at(Graphics::GetFrame());
		}

		cmd->OMSetRenderTargets(1, &rtvDescriptor, TRUE, &dsvDescriptor);
		if (m_RenderPass->GetSpecification().Operation.ColorBegin == RENDER_PASS_BEGINE_OP_CLEAR)
		{
			cmd->ClearRenderTargetView(rtvDescriptor, Math::ValuePtr(D3D12Context::Get()->GetClearColor()), 0, nullptr);
		}
		if (m_RenderPass->GetSpecification().Operation.DepthStencilBegin == RENDER_PASS_BEGINE_OP_CLEAR)
		{
			cmd->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Graphics::IsReverseDepth() ? 0 : 1.0f, 0, 0, nullptr);
		}

		D3D12_VIEWPORT							viewPort = {};
		viewPort.TopLeftX						= viewPort.TopLeftY = 0;
		viewPort.Width							= static_cast<float>(m_Specification.Width);
		viewPort.Height							= static_cast<float>(m_Specification.Height);
		viewPort.MinDepth = D3D12_MIN_DEPTH;
		viewPort.MaxDepth = D3D12_MAX_DEPTH;

		cmd->RSSetViewports(1, &viewPort);

		D3D12_RECT				rect = {};
		rect.left				= rect.top = 0;
		rect.right				= static_cast<LONG>(m_Specification.Width);
		rect.bottom				= static_cast<LONG>(m_Specification.Height);

		cmd->RSSetScissorRects(1, &rect);
	}
	void D3D12FrameBuffer::EndPresentRender(CommandBuffer* cmdBuffer)
	{
		// RenderGraph owns the swap-chain PRESENT <-> RENDER_TARGET transitions.
	}
	void D3D12FrameBuffer::CreateResources()
	{
		m_RenderTargetResources.resize(m_Specification.RenderTargets.size());
		if(m_Specification.Samples > 1)
			m_MultiSampleRenderTargetResources.resize(m_Specification.RenderTargets.size());
		m_RenderTargets.resize(m_Specification.RenderTargets.size());

		for (UINT i = 0; i < m_Specification.RenderTargets.size(); i++)
		{
			const FLOAT v[4] = { 0, 0, 0, 0 };
			const CD3DX12_CLEAR_VALUE clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.RenderTargets[i]), v);

			D3D12_RENDER_TARGET_VIEW_DESC	rtvDesc = {};
			rtvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.RenderTargets[i]);
			rtvDesc.ViewDimension			= D3D12_RTV_DIMENSION_TEXTURE2D;

			if (m_Specification.Samples > 1)
			{
				D3D12_RESOURCE_DESC multiDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.RenderTargets[i]),
					m_Specification.Width,
					m_Specification.Height,
					1, // This resource has only one texture.
					1,  // Use a single mipmap level.
					m_Specification.Samples
				);

				multiDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&multiDesc,
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&clearValue,
					IID_PPV_ARGS(m_MultiSampleRenderTargetResources[i].GetAddressOf())));

				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
				// create multi sample rtv for each frame
				for (int frame = 0; frame < Graphics::GetFrameCount(); frame++)
				{
					const CD3DX12_CPU_DESCRIPTOR_HANDLE multiSampleRtvDescriptor(m_MultiSampleRtvHeap.CpuHandles[frame], static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorIncrementSize());
					D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_MultiSampleRenderTargetResources[i].Get(), &rtvDesc, multiSampleRtvDescriptor);
				}
			}

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
				Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.RenderTargets[i]),
				m_Specification.Width,
				m_Specification.Height,
				1, // This resource has only one texture.
				1  // Use a single mipmap level.
			);

			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				&clearValue,
				IID_PPV_ARGS(m_RenderTargetResources[i].GetAddressOf())));

			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			// create rtv for each frame
			for(int frame = 0; frame < Graphics::GetFrameCount(); frame++)
			{
				const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_RtvHeap.CpuHandles[frame], static_cast<INT>(i), D3D12Context::Get()->GetRtvDescriptorIncrementSize());
				D3D12Context::Get()->GetDevice()->CreateRenderTargetView(m_RenderTargetResources[i].Get(), &rtvDesc, rtvDescriptor);
			}

			m_RenderTargets.at(i) = CreateRef<D3D12Texture2D>(m_RenderTargetResources[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		if (m_Specification.DepthStencil != FRAME_BUFFER_TEXTURE_FORMAT_NONE)
		{
			const CD3DX12_CLEAR_VALUE clearValue(Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencil), Graphics::IsReverseDepth() ? 0.0f : 1.0f, 0);
			D3D12_DEPTH_STENCIL_VIEW_DESC	dsvDesc = {};
			dsvDesc.Format					= Utils::FrameBufferTextureFormatToDXGIFormat(m_Specification.DepthStencil);
			dsvDesc.ViewDimension			= D3D12_DSV_DIMENSION_TEXTURE2D;

			if (m_Specification.Samples > 1)
			{
				D3D12_RESOURCE_DESC multiDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					m_Specification.DepthStencil == FRAME_BUFFER_TEXTURE_FORMAT_DEPTH ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS,
					m_Specification.Width,
					m_Specification.Height,
					1, // This resource has only one texture.
					1,  // Use a single mipmap level.
					m_Specification.Samples
				);

				multiDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


				D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&multiDesc,
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&clearValue,
					IID_PPV_ARGS(m_MultiSampleDepthStencilResource.GetAddressOf())));

				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
				// create multi sample dsv for each frame
				for(int frame = 0; frame < Graphics::GetFrameCount(); frame++)
				{
					const CD3DX12_CPU_DESCRIPTOR_HANDLE multiSampleDsvDescriptor(m_MultiSampleDsvHeap.CpuHandles[frame], 0, D3D12Context::Get()->GetDsvDescriptorIncrementSize());
					D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_MultiSampleDepthStencilResource.Get(), &dsvDesc, multiSampleDsvDescriptor);
				}
			}

			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
				m_Specification.DepthStencil == FRAME_BUFFER_TEXTURE_FORMAT_DEPTH ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS,
				m_Specification.Width,
				m_Specification.Height,
				1, // This resource has only one texture.
				1  // Use a single mipmap level.
			);

			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(m_DepthStencilResource.GetAddressOf())));
			
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			// create dsv for each frame
			for(int frame = 0; frame < Graphics::GetFrameCount(); frame++)
			{
				const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor(m_DsvHeap.CpuHandles[frame], 0, D3D12Context::Get()->GetDsvDescriptorIncrementSize());
				D3D12Context::Get()->GetDevice()->CreateDepthStencilView(m_DepthStencilResource.Get(), &dsvDesc, dsvDescriptor);
			}

			m_DepthStencil = CreateRef<D3D12Texture2D>(m_DepthStencilResource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}
}

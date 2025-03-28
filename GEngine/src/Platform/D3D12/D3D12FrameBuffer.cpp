#include "GEpch.h"
#include "D3D12FrameBuffer.h"
#include "D3D12Utils.h"
#include "D3D12Context.h"

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
			heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DsvHeap)));
		}
		

		CreateResources();
	}
	void D3D12FrameBuffer::CreateResources()
	{

	}
}
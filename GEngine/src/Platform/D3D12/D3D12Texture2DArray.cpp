#include "GEpch.h"
#include "Platform/D3D12/D3D12Texture2DArray.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12Utils.h"

namespace GEngine
{
	D3D12Texture2DArray::D3D12Texture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
	{
		GE_CORE_ASSERT(width > 0 && height > 0 && layers > 0, "Texture array dimensions must be non-zero.");
		m_Width = width;
		m_Height = height;
		m_Layers = layers;
		m_Format = format;
		m_MipLevels = 1;

		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(Utils::RenderImage2DFormatToDXGIFormat(format), width, height, static_cast<UINT16>(layers));
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Texture)));

		m_SrvDesc.Format = Utils::RenderImage2DFormatToDXGIFormat(format);
		m_SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		m_SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_SrvDesc.Texture2DArray.MostDetailedMip = 0;
		m_SrvDesc.Texture2DArray.MipLevels = 1;
		m_SrvDesc.Texture2DArray.FirstArraySlice = 0;
		m_SrvDesc.Texture2DArray.ArraySize = layers;
		m_SrvDesc.Texture2DArray.PlaneSlice = 0;
		m_SrvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	}

	bool D3D12Texture2DArray::operator==(const Texture2DArray& other) const
	{
		auto d3dOther = dynamic_cast<const D3D12Texture2DArray*>(&other);
		return d3dOther && m_Texture.Get() == d3dOther->m_Texture.Get();
	}

	void D3D12Texture2DArray::SetData(const void* data, uint32_t size)
	{
		const uint32_t layerSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		GE_CORE_ASSERT(size == layerSize * m_Layers, "Texture array data size does not match all layers.");
		const auto* bytes = static_cast<const uint8_t*>(data);
		for (uint32_t layer = 0; layer < m_Layers; ++layer)
			UploadLayer(bytes + layer * layerSize, layerSize, layer);
	}

	void D3D12Texture2DArray::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer)
	{
		GE_CORE_ASSERT(width == m_Width && height == m_Height && layer < m_Layers, "Texture array copy dimensions are invalid.");
		auto source = std::dynamic_pointer_cast<D3D12Texture2D>(texture);
		GE_CORE_ASSERT(source, "D3D12 texture arrays require D3D12 source textures.");

		auto command = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
		auto commandList = command->GetCommandList();
		source->TransitionResourceState(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
		CD3DX12_RESOURCE_BARRIER toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &toCopy);
		CD3DX12_TEXTURE_COPY_LOCATION destination(m_Texture.Get(), layer);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source->GetResource().Get(), 0);
		commandList->CopyTextureRegion(&destination, 0, 0, 0, &sourceLocation, nullptr);
		CD3DX12_RESOURCE_BARRIER toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &toRead);
		source->TransitionResourceState(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}

	void D3D12Texture2DArray::SetData(const void* data, uint32_t size, uint32_t layer)
	{
		UploadLayer(data, size, layer);
	}

	void D3D12Texture2DArray::UploadLayer(const void* data, uint32_t size, uint32_t layer)
	{
		GE_CORE_ASSERT(data && layer < m_Layers, "Texture-array layer is invalid.");
		const uint32_t expectedSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		GE_CORE_ASSERT(size == expectedSize, "Texture-array layer data size is invalid.");

		const UINT64 uploadSize = GetRequiredIntermediateSize(m_Texture.Get(), layer, 1);
		Microsoft::WRL::ComPtr<ID3D12Resource> upload;
		Utils::CreateBuffer(uploadSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, upload);

		D3D12_SUBRESOURCE_DATA source{};
		source.pData = data;
		source.RowPitch = static_cast<LONG_PTR>(m_Width * RenderImage2DFormatChannelSize(m_Format));
		source.SlicePitch = source.RowPitch * m_Height;

		auto command = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
		auto commandList = command->GetCommandList();
		CD3DX12_RESOURCE_BARRIER toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &toCopy);
		UpdateSubresources(commandList.Get(), m_Texture.Get(), upload.Get(), 0, layer, 1, &source);
		CD3DX12_RESOURCE_BARRIER toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &toRead);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}
}

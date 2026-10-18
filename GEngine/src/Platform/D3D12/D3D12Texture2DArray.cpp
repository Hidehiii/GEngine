#include "GEpch.h"
#include "Platform/D3D12/D3D12Texture2DArray.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"

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
		m_SubresourceStates.assign(layers, D3D12_RESOURCE_STATE_GENERIC_READ);

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

	void D3D12Texture2DArray::TransitionResourceState(
		const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
		D3D12_RESOURCE_STATES destination, const GraphicsSubresourceRange& range)
	{
		if (range.BaseMipLevel != 0 || (range.MipLevelCount != GraphicsSubresourceRange::All && range.MipLevelCount != 1))
			throw std::invalid_argument("D3D12 texture-array mip range is out of bounds.");
		const uint32_t firstLayer = range.BaseArrayLayer;
		if (firstLayer >= m_Layers)
			throw std::invalid_argument("D3D12 texture-array layer range is out of bounds.");
		const uint32_t layerCount = range.ArrayLayerCount == GraphicsSubresourceRange::All
			? m_Layers - firstLayer : range.ArrayLayerCount;
		if (layerCount == 0 || firstLayer + layerCount > m_Layers)
			throw std::invalid_argument("D3D12 texture-array layer range is out of bounds.");

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.reserve(layerCount);
		for (uint32_t layer = firstLayer; layer < firstLayer + layerCount; ++layer)
		{
			if (m_SubresourceStates[layer] == destination) continue;
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(),
				m_SubresourceStates[layer], destination, D3D12CalcSubresource(0, layer, 0, 1, m_Layers)));
			m_SubresourceStates[layer] = destination;
		}
		if (!barriers.empty()) commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
	}

	void D3D12Texture2DArray::MarkCopyQueueDecay()
	{
		std::fill(m_SubresourceStates.begin(), m_SubresourceStates.end(), D3D12_RESOURCE_STATE_COMMON);
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
		const auto subresource = D3D12CalcSubresource(0, layer, 0, 1, m_Layers);
		CD3DX12_RESOURCE_BARRIER toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), m_SubresourceStates[layer], D3D12_RESOURCE_STATE_COPY_DEST, subresource);
		commandList->ResourceBarrier(1, &toCopy);
		CD3DX12_TEXTURE_COPY_LOCATION destination(m_Texture.Get(), layer);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source->GetResource().Get(), 0);
		commandList->CopyTextureRegion(&destination, 0, 0, 0, &sourceLocation, nullptr);
		CD3DX12_RESOURCE_BARRIER toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, subresource);
		commandList->ResourceBarrier(1, &toRead);
		m_SubresourceStates[layer] = D3D12_RESOURCE_STATE_GENERIC_READ;
		source->TransitionResourceState(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}

	void D3D12Texture2DArray::SetData(const void* data, uint32_t size, uint32_t layer)
	{
		UploadLayer(data, size, layer);
	}
	void D3D12Texture2DArray::ReadLayerData(uint32_t layer, uint32_t size, void* destination)
	{
		const uint32_t rowSize = m_Width * 4;
		if (!destination || m_Format != RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM || layer >= m_Layers ||
			size != rowSize * m_Height)
			throw std::invalid_argument("D3D12 array readback requires an RGBA8 layer and its exact byte size.");
		Graphics::GetRenderDevice().GetQueue(COMMAND_BUFFER_TYPE_GRAPHICS).WaitForIdle();
		const auto description = m_Texture->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
		UINT64 requiredSize = 0;
		D3D12Context::Get()->GetDevice()->GetCopyableFootprints(&description, layer, 1, 0,
			&footprint, nullptr, nullptr, &requiredSize);
		Microsoft::WRL::ComPtr<ID3D12Resource> staging;
		Utils::CreateBuffer(requiredSize, D3D12_HEAP_TYPE_READBACK, D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_COPY_DEST, staging);
		auto command = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
		auto list = command->GetCommandList();
		const auto previousState = m_SubresourceStates[layer];
		TransitionResourceState(list, D3D12_RESOURCE_STATE_COPY_SOURCE, { 0, 1, layer, 1 });
		D3D12_TEXTURE_COPY_LOCATION sourceLocation{};
		sourceLocation.pResource = m_Texture.Get();
		sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		sourceLocation.SubresourceIndex = layer;
		D3D12_TEXTURE_COPY_LOCATION destinationLocation{};
		destinationLocation.pResource = staging.Get();
		destinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		destinationLocation.PlacedFootprint = footprint;
		list->CopyTextureRegion(&destinationLocation, 0, 0, 0, &sourceLocation, nullptr);
		TransitionResourceState(list, previousState, { 0, 1, layer, 1 });
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
		void* mapped = nullptr;
		D3D12_RANGE readRange{ 0, static_cast<SIZE_T>(requiredSize) };
		D3D12_THROW_IF_FAILED(staging->Map(0, &readRange, &mapped));
		for (uint32_t row = 0; row < m_Height; ++row)
			memcpy(static_cast<uint8_t*>(destination) + static_cast<size_t>(row) * rowSize,
				static_cast<const uint8_t*>(mapped) + footprint.Offset + static_cast<size_t>(row) * footprint.Footprint.RowPitch,
				rowSize);
		D3D12_RANGE written{ 0, 0 };
		staging->Unmap(0, &written);
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
		const auto subresource = D3D12CalcSubresource(0, layer, 0, 1, m_Layers);
		CD3DX12_RESOURCE_BARRIER toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), m_SubresourceStates[layer], D3D12_RESOURCE_STATE_COPY_DEST, subresource);
		commandList->ResourceBarrier(1, &toCopy);
		UpdateSubresources(commandList.Get(), m_Texture.Get(), upload.Get(), 0, layer, 1, &source);
		CD3DX12_RESOURCE_BARRIER toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, subresource);
		commandList->ResourceBarrier(1, &toRead);
		m_SubresourceStates[layer] = D3D12_RESOURCE_STATE_GENERIC_READ;
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}
}

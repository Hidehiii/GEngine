#include "GEpch.h"
#include "Platform/D3D12/D3D12CubeMap.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "stb_image.h"

namespace GEngine
{
	D3D12CubeMap::D3D12CubeMap(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		GE_CORE_ASSERT(width > 0 && height > 0, "Cube-map dimensions must be non-zero.");
		m_Width = width;
		m_Height = height;
		m_Format = format;
		m_GenerateMipmap = generateMipmap;
		m_MipLevels = 1;

		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(Utils::RenderImage2DFormatToDXGIFormat(format), width, height, 6);
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Texture)));

		m_SrvDesc.Format = Utils::RenderImage2DFormatToDXGIFormat(format);
		m_SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		m_SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_SrvDesc.TextureCube.MostDetailedMip = 0;
		m_SrvDesc.TextureCube.MipLevels = 1;
		m_SrvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}

	D3D12CubeMap::D3D12CubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& bottomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
		: m_Paths{ rightPath, leftPath, topPath, bottomPath, backPath, frontPath }
	{
		int width = 0, height = 0, channels = 0;
		stbi_uc* first = stbi_load(rightPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		GE_CORE_ASSERT(first, "Failed to load cube-map face.");
		D3D12CubeMap temporary(static_cast<uint32_t>(width), static_cast<uint32_t>(height), generateMipmap, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		*this = std::move(temporary);
		m_Paths = { rightPath, leftPath, topPath, bottomPath, backPath, frontPath };
		UploadFace(first, static_cast<uint32_t>(width * height * 4), 0);
		stbi_image_free(first);

		for (uint32_t face = 1; face < 6; ++face)
		{
			int faceWidth = 0, faceHeight = 0, faceChannels = 0;
			stbi_uc* data = stbi_load(m_Paths[face].c_str(), &faceWidth, &faceHeight, &faceChannels, STBI_rgb_alpha);
			GE_CORE_ASSERT(data && faceWidth == width && faceHeight == height, "Cube-map faces must have matching dimensions.");
			UploadFace(data, static_cast<uint32_t>(width * height * 4), face);
			stbi_image_free(data);
		}
	}

	std::string D3D12CubeMap::GetPath() const
	{
		return m_Paths.empty() ? std::string{} : m_Paths.front();
	}

	bool D3D12CubeMap::operator==(const CubeMap& other) const
	{
		auto d3dOther = dynamic_cast<const D3D12CubeMap*>(&other);
		return d3dOther && m_Texture.Get() == d3dOther->m_Texture.Get();
	}

	void D3D12CubeMap::SetData(const void* data, uint32_t size)
	{
		const uint32_t faceSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		GE_CORE_ASSERT(size == faceSize * 6, "Cube-map data size does not match all faces.");
		const auto* bytes = static_cast<const uint8_t*>(data);
		for (uint32_t face = 0; face < 6; ++face)
			UploadFace(bytes + face * faceSize, faceSize, face);
	}

	void D3D12CubeMap::SetData(const void* data, uint32_t size, CubeMapFace face)
	{
		UploadFace(data, size, static_cast<uint32_t>(face));
	}

	void D3D12CubeMap::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face)
	{
		GE_CORE_ASSERT(width == m_Width && height == m_Height && static_cast<uint32_t>(face) < 6, "Cube-map copy dimensions are invalid.");
		auto source = std::dynamic_pointer_cast<D3D12Texture2D>(texture);
		GE_CORE_ASSERT(source, "D3D12 cube maps require D3D12 source textures.");
		auto command = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
		auto commandList = command->GetCommandList();
		source->TransitionResourceState(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
		CD3DX12_RESOURCE_BARRIER toCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &toCopy);
		CD3DX12_TEXTURE_COPY_LOCATION destination(m_Texture.Get(), static_cast<uint32_t>(face));
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(source->GetResource().Get(), 0);
		commandList->CopyTextureRegion(&destination, 0, 0, 0, &sourceLocation, nullptr);
		CD3DX12_RESOURCE_BARRIER toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &toRead);
		source->TransitionResourceState(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}

	void D3D12CubeMap::UploadFace(const void* data, uint32_t size, uint32_t face)
	{
		GE_CORE_ASSERT(data && face < 6, "Cube-map face is invalid.");
		const uint32_t expectedSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		GE_CORE_ASSERT(size == expectedSize, "Cube-map face data size is invalid.");
		const UINT64 uploadSize = GetRequiredIntermediateSize(m_Texture.Get(), face, 1);
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
		UpdateSubresources(commandList.Get(), m_Texture.Get(), upload.Get(), 0, face, 1, &source);
		CD3DX12_RESOURCE_BARRIER toRead = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &toRead);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}
}

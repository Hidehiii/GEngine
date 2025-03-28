#include "GEpch.h"
#include "D3D12Texture2D.h"
#include "stb_image.h"
#include "D3D12Utils.h"

namespace GEngine
{
	D3D12Texture2D::D3D12Texture2D(const std::string& path)
	{
		m_Path = path;

		int	width, height, channels;
		stbi_uc* data;
		stbi_set_flip_vertically_on_load(1);
		data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		if (channels == 4)
		{
			m_Format = RenderImage2DFormat::RGBA8F;
		}
		else if (channels == 3)
		{
			m_Format = RenderImage2DFormat::RGB8F;
		}
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}

		Utils::CreateTextures(width, height, 1, m_MipLevels,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			m_State,
			m_Texture);
	}
	D3D12Texture2D::D3D12Texture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
	}
	D3D12Texture2D::D3D12Texture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
	}
	D3D12Texture2D::~D3D12Texture2D()
	{
	}
	void D3D12Texture2D::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
	}
	void D3D12Texture2D::SetData(const void* data, uint32_t size)
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		Utils::CreateBuffers(uploadBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadBuffer);
		Utils::CopyDataToTextures(m_Width, m_Height, RenderImage2DFormatChannelSize(m_Format), m_Texture, uploadBuffer, data);
	}
	void D3D12Texture2D::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height)
	{
	}
}

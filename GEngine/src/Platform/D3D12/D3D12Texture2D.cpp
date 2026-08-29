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
		data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		m_Format = RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM;
		m_DxgiFormat = Utils::RenderImage2DFormatToDXGIFormat(m_Format);
		// Mip generation is not implemented by this backend yet.  Expose only the
		// initialized level instead of allowing sampling from undefined subresources.
		m_MipLevels = 1;

		Utils::CreateTexture(width, height, 1, m_MipLevels,
			m_DxgiFormat,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			m_State,
			m_Texture);
		InitializeShaderResourceView();
		SetData(data, static_cast<uint32_t>(width * height * 4));
		stbi_image_free(data);
	}
	D3D12Texture2D::D3D12Texture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		m_Width		= width;
		m_Height	= height;
		m_Format	= format;
		m_DxgiFormat = Utils::RenderImage2DFormatToDXGIFormat(m_Format);
		m_MipLevels = 1;
		Utils::CreateTexture(width, height, 1, m_MipLevels,
			m_DxgiFormat,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			m_State,
			m_Texture);
		InitializeShaderResourceView();
	}
	D3D12Texture2D::D3D12Texture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;
		m_DxgiFormat = Utils::RenderImage2DFormatToDXGIFormat(m_Format);
		m_MipLevels = 1;
		Utils::CreateTexture(width, height, 1, m_MipLevels,
			m_DxgiFormat,
			D3D12_RESOURCE_FLAG_NONE,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			m_State,
			m_Texture);
		InitializeShaderResourceView();
		SetData(data, size);
	}
	D3D12Texture2D::D3D12Texture2D(Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_RESOURCE_STATES state)
	{
		m_Texture = texture;
		m_State = state;
		if (m_Texture)
		{
			const auto desc = m_Texture->GetDesc();
			m_Width = static_cast<uint32_t>(desc.Width);
			m_Height = desc.Height;
			m_MipLevels = desc.MipLevels;
			m_DxgiFormat = desc.Format;
			m_Format = RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM;
			InitializeShaderResourceView();
		}
	}
	D3D12Texture2D::~D3D12Texture2D()
	{
	}
	void D3D12Texture2D::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
	}
	void D3D12Texture2D::InitializeShaderResourceView()
	{
		m_SRVDesc.Format					= m_DxgiFormat != DXGI_FORMAT_UNKNOWN ? m_DxgiFormat : Utils::RenderImage2DFormatToDXGIFormat(m_Format);
		m_SRVDesc.ViewDimension				= D3D12_SRV_DIMENSION_TEXTURE2D;
		m_SRVDesc.Shader4ComponentMapping	= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_SRVDesc.Texture2D.MipLevels		= m_MipLevels;
	}
	void D3D12Texture2D::SetData(const void* data, uint32_t size)
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		Utils::CreateBuffer(uploadBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadBuffer);
		TransitionResourceState(D3D12_RESOURCE_STATE_COPY_DEST);
		Utils::CopyDataToTextures(m_Width, m_Height, RenderImage2DFormatChannelSize(m_Format), m_Texture, uploadBuffer, data);
		TransitionResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	void D3D12Texture2D::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height)
	{
	}
	void D3D12Texture2D::TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_RESOURCE_STATES dst)
	{
		Utils::TransitionResourceState(commandList, m_Texture, m_State, dst);
		m_State = dst;
	}
	void D3D12Texture2D::TransitionResourceState(D3D12_RESOURCE_STATES dst)
	{
		Utils::TransitionResourceState(m_Texture, m_State, dst);
		m_State = dst;
	}
}

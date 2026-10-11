#include "GEpch.h"
#include "Platform/D3D12/D3D12Texture2D.h"
#include "Platform/D3D12/D3D12Context.h"
#include "stb_image.h"
#include "Platform/D3D12/D3D12Utils.h"

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
		: D3D12Texture2D(width, height, format, 0)
	{
	}
	D3D12Texture2D::D3D12Texture2D(uint32_t width, uint32_t height, RenderImage2DFormat format, uint32_t mipLevelCount)
	{
		m_Width		= width;
		m_Height	= height;
		m_Format	= format;
		m_DxgiFormat = Utils::RenderImage2DFormatToDXGIFormat(m_Format);
		m_MipLevels = mipLevelCount != 0 ? mipLevelCount : 1;
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
		EnsureSubresourceStates();
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.reserve(m_SubresourceStates.size());
		for (uint32_t subresource = 0; subresource < m_SubresourceStates.size(); ++subresource)
		{
			if (m_SubresourceStates[subresource] == dst) continue;
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), m_SubresourceStates[subresource], dst, subresource));
			m_SubresourceStates[subresource] = dst;
		}
		if (!barriers.empty()) commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
		m_State = dst;
	}
	void D3D12Texture2D::TransitionResourceState(D3D12_RESOURCE_STATES dst)
	{
		auto command = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
		TransitionResourceState(command->GetCommandList(), dst);
		D3D12Context::Get()->EndSingleTimeGraphicsCommand(command);
	}
	void D3D12Texture2D::TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
		D3D12_RESOURCE_STATES dst, const GraphicsSubresourceRange& range)
	{
		EnsureSubresourceStates();
		const auto desc = m_Texture->GetDesc();
		const uint32_t mipCount = desc.MipLevels;
		const uint32_t firstMip = range.BaseMipLevel;
		const uint32_t mipCountToTransition = range.MipLevelCount == GraphicsSubresourceRange::All
			? mipCount - firstMip : range.MipLevelCount;
		if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || range.BaseArrayLayer != 0 ||
			range.ArrayLayerCount != GraphicsSubresourceRange::All && range.ArrayLayerCount != 1 ||
			firstMip >= mipCount || mipCountToTransition == 0 || firstMip + mipCountToTransition > mipCount)
			throw std::invalid_argument("D3D12 texture transition range is unsupported or out of bounds.");

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.reserve(mipCountToTransition);
		for (uint32_t mip = firstMip; mip < firstMip + mipCountToTransition; ++mip)
		{
			const uint32_t subresource = D3D12CalcSubresource(mip, 0, 0, desc.MipLevels, desc.DepthOrArraySize);
			if (m_SubresourceStates[subresource] == dst) continue;
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), m_SubresourceStates[subresource], dst, subresource));
			m_SubresourceStates[subresource] = dst;
		}
		if (!barriers.empty()) commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
		m_State = dst;
	}
	void D3D12Texture2D::EnsureSubresourceStates()
	{
		if (!m_Texture || !m_SubresourceStates.empty()) return;
		const auto desc = m_Texture->GetDesc();
		D3D12_FEATURE_DATA_FORMAT_INFO formatInfo{};
		formatInfo.Format = desc.Format;
		uint32_t planeCount = 1;
		if (SUCCEEDED(D3D12Context::Get()->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO,
			&formatInfo, sizeof(formatInfo))))
			planeCount = std::max(1u, static_cast<uint32_t>(formatInfo.PlaneCount));
		const auto count = static_cast<size_t>(desc.MipLevels) * desc.DepthOrArraySize * planeCount;
		m_SubresourceStates.assign(count, m_State);
	}
}

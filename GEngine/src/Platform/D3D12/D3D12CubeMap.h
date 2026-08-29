#pragma once

#include "GEngine/Graphics/Texture.h"
#include <d3d12.h>
#include <wrl.h>

namespace GEngine
{
	class GENGINE_API D3D12CubeMap : public CubeMap
	{
	public:
		D3D12CubeMap(uint32_t width, uint32_t height, bool generateMipmap = false, RenderImage2DFormat format = RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
		D3D12CubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& bottomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap = false);
		virtual ~D3D12CubeMap() override = default;

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override {}
		virtual std::string GetPath() const override;
		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const void* data, uint32_t size, CubeMapFace face) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face) override;
		virtual bool operator==(const CubeMap& other) const override;

		ID3D12Resource* GetResource() const { return m_Texture.Get(); }
		const D3D12_SHADER_RESOURCE_VIEW_DESC& GetShaderResourceViewDesc() const { return m_SrvDesc; }

	private:
		void UploadFace(const void* data, uint32_t size, uint32_t face);

	private:
		std::vector<std::string> m_Paths;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SrvDesc{};
	};
}

#pragma once
#include "GEngine/Graphics/Texture.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>

namespace GEngine
{
	class GENGINE_API D3D12Texture2D : public Texture2D
	{
	public:
		D3D12Texture2D(const std::string& path);
		D3D12Texture2D(uint32_t width, uint32_t height, RenderImage2DFormat format = RENDER_IMAGE_2D_FORMAT_RGBA8F);
		D3D12Texture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format = RENDER_IMAGE_2D_FORMAT_RGBA8F);
		D3D12Texture2D(Microsoft::WRL::ComPtr<ID3D12Resource>& texture, D3D12_RESOURCE_STATES state);
		virtual ~D3D12Texture2D();

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override;

		virtual std::string GetPath() const override { return m_Path; };

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height) override;

		// DX的相等不是比较ID，而是比较是否是同一个对象
		virtual bool operator==(const Texture2D& other) const override { return m_Texture == ((D3D12Texture2D&)other).m_Texture; };

		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_RESOURCE_STATES dst);
		void TransitionResourceState(D3D12_RESOURCE_STATES dst);
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>	m_Texture;
		D3D12_RESOURCE_STATES					m_State = D3D12_RESOURCE_STATE_COPY_DEST;
	};
}



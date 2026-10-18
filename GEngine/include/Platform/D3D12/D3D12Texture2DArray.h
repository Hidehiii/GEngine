#pragma once

#include "GEngine/Graphics/Texture.h"
#include <d3d12.h>
#include <wrl.h>
#include <vector>

namespace GEngine
{
	class GENGINE_API D3D12Texture2DArray : public Texture2DArray
	{
	public:
		D3D12Texture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format);
		virtual ~D3D12Texture2DArray() override = default;

		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override {}
		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer) override;
		virtual void SetData(const void* data, uint32_t size, uint32_t layer) override;
		virtual void ReadLayerData(uint32_t layer, uint32_t size, void* destination) override;
		virtual std::string GetPath() const override { return {}; }
		virtual bool operator==(const Texture2DArray& other) const override;

		ID3D12Resource* GetResource() const { return m_Texture.Get(); }
		void TransitionResourceState(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
			D3D12_RESOURCE_STATES destination, const GraphicsSubresourceRange& range);
		void MarkCopyQueueDecay();
		const D3D12_SHADER_RESOURCE_VIEW_DESC& GetShaderResourceViewDesc() const { return m_SrvDesc; }

	private:
		void UploadLayer(const void* data, uint32_t size, uint32_t layer);

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Texture;
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SrvDesc{};
		std::vector<D3D12_RESOURCE_STATES> m_SubresourceStates;
	};
}

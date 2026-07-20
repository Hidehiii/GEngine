#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/VertexBuffer.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>

namespace GEngine
{
	class D3D12IndexBuffer;

	class GENGINE_API D3D12VertexBuffer : public VertexBuffer
	{
	public:
		D3D12VertexBuffer(uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		D3D12VertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance = 0, VertexTopology type = VERTEX_TOPOLOGY_TRIANGLE);
		virtual ~D3D12VertexBuffer();

		virtual void SetVertexData(const void* data, uint32_t size) override;
		virtual void SetInstanceData(const void* data, uint32_t size) override;
		virtual void SetIndexBuffer(const Ref<GEngine::IndexBuffer>& indexBuffer) override;

		virtual const Ref<IndexBuffer>&				GetIndexBuffer() const override { return std::dynamic_pointer_cast<IndexBuffer>(m_IndexBuffer); }
		virtual VertexTopology						GetVertexTopologyType() override { return m_TopologyType; }

		virtual uint32_t GetIndexCount() const override;

		Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer() const { return m_VertexBuffer; }
		Microsoft::WRL::ComPtr<ID3D12Resource> GetInstanceBuffer() const { return m_InstanceBuffer; }

		const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputElementDescs() const { return m_InputElementDescs; }
		const D3D12_VERTEX_BUFFER_VIEW&			GetVertexBufferView() const { return m_VertexBufferView; }
		bool									HasInstanceBuffer() const { return m_InstanceBuffer != nullptr; }

		bool operator==(const D3D12VertexBuffer& other) const
		{
			return m_VertexBuffer.Get() == other.m_VertexBuffer.Get() &&
				m_InstanceBuffer.Get() == other.m_InstanceBuffer.Get();
		}
	protected:
		virtual void Bind(CommandBuffer* cmd) const override;

		virtual void SetShaderAndInputLayout(const Ref<Shader>& shader, uint32_t pass) override;
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>		m_VertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource>		m_InstanceBuffer = nullptr;
		D3D12_VERTEX_BUFFER_VIEW					m_VertexBufferView;
		D3D12_VERTEX_BUFFER_VIEW					m_InstanceBufferView;
		std::vector<D3D12_INPUT_ELEMENT_DESC>		m_InputElementDescs;

		Ref<D3D12IndexBuffer>						m_IndexBuffer = nullptr;
	};

	class GENGINE_API D3D12IndexBuffer : public IndexBuffer
	{
	public:
		D3D12IndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~D3D12IndexBuffer();

		bool operator==(const D3D12IndexBuffer& other) const
		{
			return m_IndexBuffer.Get() == other.m_IndexBuffer.Get();
		}
	protected:
		virtual void Bind(CommandBuffer* cmd) const override;
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>		m_IndexBuffer = nullptr;
		D3D12_INDEX_BUFFER_VIEW						m_IndexBufferView;

		friend class D3D12VertexBuffer;
	};
}



#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "Platform/D3D12/D3D12VertexBuffer.h"
#include "Platform/D3D12/D3D12RenderPass.h"
#include "Platform/D3D12/D3D12Material.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>

namespace GEngine
{ 
	class GENGINE_API D3D12GraphicsPipeline : public GraphicsPipeline
	{
	public:
		D3D12GraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
		virtual ~D3D12GraphicsPipeline() override;

		virtual Ref<VertexBuffer>	GetVertexBuffer() override;
		virtual void				SetVertexBuffer(Ref<VertexBuffer>& buffer) override;

		virtual Ref<Material>	GetMaterial() override;
		virtual void			SetMaterial(Ref<Material>& material) override;

		bool operator==(const D3D12GraphicsPipeline& other) const
		{
			return m_Material == other.m_Material && m_VertexBuffer == other.m_VertexBuffer;
		}
	protected:
		virtual void Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, uint32_t pass, uint32_t instanceCount = 1, uint32_t indexCount = 0) override;

		// use to identify whether the current pass and pipeline are compatible
		struct D3D12GraphicsPipelineInfo
		{
			Microsoft::WRL::ComPtr<ID3D12PipelineState>	PipelineState;
			Ref<D3D12RenderPass>						RenderPass;
			uint32_t									Pass;
		};

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState(const Ref<FrameBuffer>& frameBuffer, const uint32_t& pass);
		void PrepareRender(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, const uint32_t& pass);
	private:
		Ref<D3D12Material>						m_Material;
		Ref<D3D12VertexBuffer>					m_VertexBuffer;
		std::vector<D3D12GraphicsPipelineInfo>	m_PipelineStates;

		bool									m_RecreatePipelineState = false;
	};
}



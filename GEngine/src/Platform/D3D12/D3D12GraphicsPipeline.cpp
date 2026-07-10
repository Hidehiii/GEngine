#include "GEpch.h"
#include "D3D12GraphicsPipeline.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12FrameBuffer.h"

namespace GEngine
{
	D3D12GraphicsPipeline::D3D12GraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
	{
		m_Material = std::dynamic_pointer_cast<D3D12Material>(material);
		m_VertexBuffer = std::dynamic_pointer_cast<D3D12VertexBuffer>(vertexBuffer);
	}

	D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
	{
		for(auto& pipelineInfo : m_PipelineStates)
		{
			if (pipelineInfo.PipelineState)
			{
				pipelineInfo.PipelineState.Reset();
			}
		}
	}

	Ref<VertexBuffer> D3D12GraphicsPipeline::GetVertexBuffer()
	{
		return m_VertexBuffer;
	}

	void D3D12GraphicsPipeline::SetVertexBuffer(Ref<VertexBuffer>& buffer)
	{
		m_VertexBuffer = std::dynamic_pointer_cast<D3D12VertexBuffer>(buffer);
		m_RecreatePipelineState = true;
	}

	Ref<Material> D3D12GraphicsPipeline::GetMaterial()
	{
		return m_Material;
	}

	void D3D12GraphicsPipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<D3D12Material>(material);
		m_RecreatePipelineState = true;
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> D3D12GraphicsPipeline::GetPipelineState(const Ref<FrameBuffer>& frameBuffer, const uint32_t& pass)
	{
		for(int i = 0; i < m_PipelineStates.size(); i++)
		{
			if(m_PipelineStates.at(i).RenderPass == std::dynamic_pointer_cast<D3D12RenderPass>(frameBuffer->GetRenderPass()) &&
				m_PipelineStates.at(i).Pass == pass)
			{
				return m_PipelineStates.at(i).PipelineState;
			}
		}

		auto shader = std::dynamic_pointer_cast<D3D12Shader>(m_Material->GetShader());

		CD3DX12_RASTERIZER_DESC				rasterDesc(D3D12_DEFAULT);
		rasterDesc.CullMode					= Utils::CullModeToD3D12CullMode(m_Material->GetCull(pass));
		rasterDesc.FillMode					= D3D12_FILL_MODE_SOLID;
		rasterDesc.FrontCounterClockwise	= TRUE;
		rasterDesc.DepthClipEnable			= FALSE;
		rasterDesc.MultisampleEnable		= TRUE;

		D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
		renderTargetBlendDesc.BlendEnable = TRUE;
		renderTargetBlendDesc.SrcBlend = Utils::BlendFactorToD3D12Blend(m_Material->GetBlendColorSrc(pass));
		renderTargetBlendDesc.DestBlend = Utils::BlendFactorToD3D12Blend(m_Material->GetBlendColorDst(pass));

		CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
		blendDesc.RenderTarget

		return Microsoft::WRL::ComPtr<ID3D12PipelineState>();
	}
}
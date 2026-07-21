#include "GEpch.h"
#include "D3D12GraphicsPipeline.h"
#include "Platform/D3D12/D3D12Utils.h"
#include "Platform/D3D12/D3D12Context.h"
#include "Platform/D3D12/D3D12CommandBuffer.h"
#include "Platform/D3D12/D3D12FrameBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Shader.h"

namespace GEngine
{
	D3D12GraphicsPipeline::D3D12GraphicsPipeline(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer)
	{
		m_Material = std::dynamic_pointer_cast<D3D12Material>(material);
		m_VertexBuffer = std::dynamic_pointer_cast<D3D12VertexBuffer>(vertexBuffer);
	}

	D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
	{
		for (auto& info : m_PipelineStates)
		{
			if (info.PipelineState)
				info.PipelineState.Reset();
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

	void D3D12GraphicsPipeline::PrepareRender(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, const uint32_t& pass)
	{
		m_Material->Update(cmdBuffer, pass);

		auto d3dCmdBuffer = static_cast<D3D12CommandBuffer*>(cmdBuffer);
		auto cmdList = d3dCmdBuffer->GetCommandList();

		// recreate flag: clear all cached pipelines, then lazy-create through GetPipelineState
		if (m_RecreatePipelineState)
		{
			for (auto& info : m_PipelineStates)
			{
				if (info.PipelineState)
					info.PipelineState.Reset();
			}
			m_PipelineStates.clear();
			m_RecreatePipelineState = false;
		}

		auto pso = GetPipelineState(frameBuffer, pass);
		cmdList->SetPipelineState(pso.Get());

		auto shader = std::dynamic_pointer_cast<D3D12Shader>(m_Material->GetShader());
		cmdList->SetGraphicsRootSignature(shader->GetRootSignatures()[pass].Get());

		//TODO: heap set in graphics present, set once in one frame, not per pipeline

		auto& heapInfo = m_Material->GetCbvSrvUavHeaps()[pass];
		if (heapInfo.IsValid())
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(heapInfo.GpuHandles[Graphics::GetFrame()]);
			cmdList->SetGraphicsRootDescriptorTable(0, gpuHandle);
		}

		m_VertexBuffer->Bind(cmdBuffer);

		cmdList->IASetPrimitiveTopology(
			Utils::VertexTopologyToD3D12PrimitiveTopology(m_VertexBuffer->GetVertexTopologyType()));
	}

	void D3D12GraphicsPipeline::Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, uint32_t pass, uint32_t instanceCount, uint32_t indexCount)
	{
		m_VertexBuffer->CheckInputLayout(m_Material->GetShader(), pass);
		PrepareRender(cmdBuffer, frameBuffer, pass);

		indexCount = indexCount > 0 ? indexCount : m_VertexBuffer->GetIndexCount();
		instanceCount = instanceCount > 0 ? instanceCount : m_VertexBuffer->GetInstanceCount();

		auto cmdList = static_cast<D3D12CommandBuffer*>(cmdBuffer)->GetCommandList();
		if (indexCount == 0)
		{
			uint32_t vertexCount = m_VertexBuffer->GetVertexCount();
			cmdList->DrawInstanced(vertexCount, instanceCount, 0, 0);
		}
		else
		{
			cmdList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
		}
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> D3D12GraphicsPipeline::GetPipelineState(const Ref<FrameBuffer>& frameBuffer, const uint32_t& pass)
	{
		for (int i = 0; i < m_PipelineStates.size(); i++)
		{
			if (m_PipelineStates.at(i).RenderPass == std::dynamic_pointer_cast<D3D12RenderPass>(frameBuffer->GetRenderPass()) &&
				m_PipelineStates.at(i).Pass == pass)
			{
				return m_PipelineStates.at(i).PipelineState;
			}
		}

		auto shader = std::dynamic_pointer_cast<D3D12Shader>(m_Material->GetShader());
		GE_CORE_ASSERT(shader, "Shader is null!");

		auto d3dFrameBuffer = std::dynamic_pointer_cast<D3D12FrameBuffer>(frameBuffer);
		GE_CORE_ASSERT(d3dFrameBuffer, "FrameBuffer is null!");

		// --- shader bytecodes ---
		D3D12_SHADER_BYTECODE vsBytecode = {};
		D3D12_SHADER_BYTECODE psBytecode = {};
		D3D12_SHADER_BYTECODE hsBytecode = {};
		D3D12_SHADER_BYTECODE dsBytecode = {};
		D3D12_SHADER_BYTECODE gsBytecode = {};
		D3D12_SHADER_BYTECODE asBytecode = {};
		D3D12_SHADER_BYTECODE msBytecode = {};

		for (auto&& [stage, byteCode] : shader->GetByteCodes().at(pass))
		{
			if (stage == SHADER_STAGE_VERTEX)           vsBytecode = byteCode;
			else if (stage == SHADER_STAGE_FRAGMENT)     psBytecode = byteCode;
			else if (stage == SHADER_STAGE_HULL)         hsBytecode = byteCode;
			else if (stage == SHADER_STAGE_DOMAIN)       dsBytecode = byteCode;
			else if (stage == SHADER_STAGE_GEOMETRY)     gsBytecode = byteCode;
			else if (stage == SHADER_STAGE_AMPLIFICATION) asBytecode = byteCode;
			else if (stage == SHADER_STAGE_MESH)         msBytecode = byteCode;
		}

		// --- vertex input layout (from D3D12VertexBuffer::SetShaderAndInputLayout) ---
		const auto& inputElements = m_VertexBuffer->GetInputElementDescs();

		D3D12_INPUT_LAYOUT_DESC 			inputLayoutDesc = {};
		inputLayoutDesc.pInputElementDescs 	= inputElements.data();
		inputLayoutDesc.NumElements 		= (UINT)inputElements.size();

		// --- rasterizer ---
		D3D12_RASTERIZER_DESC 					rasterizerDesc = {};
		rasterizerDesc.FillMode 				= D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode 				= Utils::CullModeToD3D12CullMode(m_Material->GetCull(pass));
		rasterizerDesc.FrontCounterClockwise 	= TRUE;
		rasterizerDesc.DepthBias 				= D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp 			= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias 	= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable 			= TRUE;
		rasterizerDesc.MultisampleEnable 		= FALSE;
		rasterizerDesc.ForcedSampleCount 		= 0;

		// --- blend (per RT) ---
		D3D12_BLEND_DESC						blendDescNew = {};
		blendDescNew.AlphaToCoverageEnable		= FALSE;
		blendDescNew.IndependentBlendEnable		= FALSE;

		D3D12_RENDER_TARGET_BLEND_DESC			rtBlendDesc = {};
		rtBlendDesc.BlendEnable					= (m_Material->GetBlendColor(pass) != BLEND_MODE_NONE) ? TRUE : FALSE;
		rtBlendDesc.SrcBlend					= Utils::BlendFactorToD3D12Blend(m_Material->GetBlendColorSrc(pass));
		rtBlendDesc.DestBlend					= Utils::BlendFactorToD3D12Blend(m_Material->GetBlendColorDst(pass));
		rtBlendDesc.BlendOp						= Utils::BlendModeToD3D12BlendOp(m_Material->GetBlendColor(pass));
		rtBlendDesc.SrcBlendAlpha				= Utils::BlendFactorToD3D12Blend(m_Material->GetBlendAlphaSrc(pass));
		rtBlendDesc.DestBlendAlpha				= Utils::BlendFactorToD3D12Blend(m_Material->GetBlendAlphaDst(pass));
		rtBlendDesc.BlendOpAlpha				= Utils::BlendModeToD3D12BlendOp(m_Material->GetBlendAlpha(pass));
		rtBlendDesc.RenderTargetWriteMask		= m_Material->GetColorMask(pass);

		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			blendDescNew.RenderTarget[i] = rtBlendDesc;

		D3D12_DEPTH_STENCIL_DESC				depthStencilDesc = {};
		depthStencilDesc.DepthEnable			= TRUE;
		depthStencilDesc.DepthWriteMask			= m_Material->GetEnableDepthWrite(pass) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc				= Utils::CompareOperationToD3D12ComparisonFunc(m_Material->GetDepthTestOp(pass));
		depthStencilDesc.StencilEnable			= FALSE;

		DXGI_SAMPLE_DESC						sampleDesc = {};
		sampleDesc.Count						= d3dFrameBuffer->GetSpecification().Samples;
		sampleDesc.Quality						= 0;

		auto& rootSignatures = shader->GetRootSignatures();
		GE_CORE_ASSERT(pass < rootSignatures.size(), "shader pass out of range");

		UINT numRTs = d3dFrameBuffer->GetRenderTargetCount();
		DXGI_FORMAT rtvFormats[8] = {};
		for (UINT i = 0; i < numRTs && i < 8; ++i)
			rtvFormats[i] = Utils::FrameBufferTextureFormatToDXGIFormat(d3dFrameBuffer->GetSpecification().RenderTargets[i]);
		DXGI_FORMAT dsvFormat = Utils::FrameBufferTextureFormatToDXGIFormat(d3dFrameBuffer->GetSpecification().DepthStencil);

				
		if (m_Material->GetShader()->IsMeshPass(pass))
		{
			// ---- mesh shader pipeline (through stream desc) ----
			D3DX12_MESH_SHADER_PIPELINE_STATE_DESC meshDesc = {};
			meshDesc.pRootSignature			= rootSignatures[pass].Get();
			meshDesc.AS						= asBytecode;
			meshDesc.MS						= msBytecode;
			meshDesc.PS						= psBytecode;
			meshDesc.BlendState				= blendDescNew;
			meshDesc.SampleMask				= UINT_MAX;
			meshDesc.RasterizerState		= rasterizerDesc;
			meshDesc.DepthStencilState		= depthStencilDesc;
			meshDesc.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			meshDesc.NumRenderTargets		= numRTs;
			memcpy(meshDesc.RTVFormats, rtvFormats, sizeof(rtvFormats));
			meshDesc.DSVFormat				= dsvFormat;
			meshDesc.SampleDesc				= sampleDesc;

			CD3DX12_PIPELINE_STATE_STREAM3	stream(meshDesc);
			D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
			streamDesc.SizeInBytes			= sizeof(stream);
			streamDesc.pPipelineStateSubobjectStream = &stream;

			Microsoft::WRL::ComPtr<ID3D12Device2> device2;
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState; 
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->QueryInterface(IID_PPV_ARGS(&device2)));
			D3D12_THROW_IF_FAILED(device2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipelineState)));

			D3D12GraphicsPipelineInfo info;
			info.PipelineState = pipelineState;
			info.RenderPass = std::dynamic_pointer_cast<D3D12RenderPass>(frameBuffer->GetRenderPass());
			info.Pass = pass;
			m_PipelineStates.push_back(info);

			return pipelineState;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC		psoDesc = {};
		psoDesc.pRootSignature					= rootSignatures[pass].Get();
		psoDesc.VS								= vsBytecode;
		psoDesc.PS								= psBytecode;
		psoDesc.DS								= dsBytecode;
		psoDesc.HS								= hsBytecode;
		psoDesc.GS								= gsBytecode;
		psoDesc.BlendState						= blendDescNew;
		psoDesc.SampleMask						= UINT_MAX;
		psoDesc.RasterizerState					= rasterizerDesc;
		psoDesc.DepthStencilState				= depthStencilDesc;
		psoDesc.InputLayout						= inputLayoutDesc;
		psoDesc.PrimitiveTopologyType			= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets				= numRTs;
		memcpy(psoDesc.RTVFormats, rtvFormats, sizeof(rtvFormats));
		psoDesc.DSVFormat						= dsvFormat;
		psoDesc.SampleDesc						= sampleDesc;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
		D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

		D3D12GraphicsPipelineInfo				info;
		info.PipelineState						= pipelineState;
		info.RenderPass							= std::dynamic_pointer_cast<D3D12RenderPass>(frameBuffer->GetRenderPass());
		info.Pass								= pass;
		m_PipelineStates.push_back(info);

		return pipelineState;
	}
}

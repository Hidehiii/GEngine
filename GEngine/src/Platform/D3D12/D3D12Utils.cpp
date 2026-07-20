#include "GEpch.h"
#include "D3D12Utils.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	namespace Utils
	{
		void CreateTexture(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS resourceFlag, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_DIMENSION dimesion, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& texture)
		{
			D3D12_RESOURCE_DESC				textureDesc = {};
			textureDesc.MipLevels			= mipLevel;
			textureDesc.Format				= format;
			textureDesc.Width				= width;
			textureDesc.Height				= height;
			textureDesc.Flags				= resourceFlag;
			textureDesc.DepthOrArraySize	= depth;
			textureDesc.SampleDesc.Count	= 1;
			textureDesc.SampleDesc.Quality	= 0;
			textureDesc.Dimension			= dimesion;

			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				heapFlag,
				&textureDesc,
				state,
				nullptr,
				IID_PPV_ARGS(&texture)
			));
		}
		void CreateBuffer(uint32_t size, D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS flag, D3D12_RESOURCE_STATES state, Microsoft::WRL::ComPtr<ID3D12Resource>& buffer)
		{
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(heapType),
				flag,
				&CD3DX12_RESOURCE_DESC::Buffer(size),
				state,
				nullptr,
				IID_PPV_ARGS(&buffer)
			));
		}
		void CopyDataToTextures(uint32_t width, uint32_t height, uint32_t pixelSize, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const void* data)
		{
			D3D12_SUBRESOURCE_DATA		textureData{};
			textureData.pData			= data;
			textureData.RowPitch		= width * pixelSize;
			textureData.SlicePitch		= textureData.RowPitch * height;

			auto dxCmd = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();

			UpdateSubresources(dxCmd->GetCommandList().Get(), texture.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);

			D3D12Context::Get()->EndSingleTimeGraphicsCommand(dxCmd);
		}
		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList, Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
		{
			if(src != dst)
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), src, dst));
		}
		void TransitionResourceState(Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES src, D3D12_RESOURCE_STATES dst)
		{
			auto dxCmd = D3D12Context::Get()->BeginSingleTimeGraphicsCommand();
			TransitionResourceState(dxCmd->GetCommandList(), resource, src, dst);
			D3D12Context::Get()->EndSingleTimeGraphicsCommand(dxCmd);
		}
		void CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = numDescriptors;
			heapDesc.Type			= type;
			heapDesc.Flags			= flags;
			D3D12_THROW_IF_FAILED(D3D12Context::Get()->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)));
		}
		HANDLE& CreateFenceEvent(LPSECURITY_ATTRIBUTES IpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR IpName)
		{
			HANDLE event = CreateEvent(IpEventAttributes, bManualReset, bInitialState, IpName);
			if (event == nullptr)
			{
				D3D12_THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
			}
			return event;
		}
		DXGI_FORMAT FrameBufferTextureFormatToDXGIFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FRAME_BUFFER_TEXTURE_FORMAT_NONE:
				return DXGI_FORMAT_UNKNOWN;
			case FRAME_BUFFER_TEXTURE_FORMAT_RGBA8:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32F:
				return DXGI_FORMAT_R32_FLOAT;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16F:
				return DXGI_FORMAT_R16G16_FLOAT;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32I:
				return DXGI_FORMAT_R32_SINT;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16I:
				return DXGI_FORMAT_R16G16_SINT;
			case FRAME_BUFFER_TEXTURE_FORMAT_R32UI:
				return DXGI_FORMAT_R32_UINT;
			case FRAME_BUFFER_TEXTURE_FORMAT_RG16UI:
				return DXGI_FORMAT_R16G16_UINT;
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case FRAME_BUFFER_TEXTURE_FORMAT_DEPTH:
				return DXGI_FORMAT_D32_FLOAT;
			default:
				break;
			}
			return DXGI_FORMAT();
		}
		FrameBufferTextureFormat DXGIFormatToFrameBufferTextureFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_UNKNOWN:			return FRAME_BUFFER_TEXTURE_FORMAT_NONE;
			case DXGI_FORMAT_R8G8B8A8_UNORM:	return FRAME_BUFFER_TEXTURE_FORMAT_RGBA8;
			case DXGI_FORMAT_R32_FLOAT:			return FRAME_BUFFER_TEXTURE_FORMAT_R32F;
			case DXGI_FORMAT_R16G16_FLOAT:		return FRAME_BUFFER_TEXTURE_FORMAT_RG16F;
			case DXGI_FORMAT_R32_SINT:			return FRAME_BUFFER_TEXTURE_FORMAT_R32I;
			case DXGI_FORMAT_R16G16_SINT:		return FRAME_BUFFER_TEXTURE_FORMAT_RG16I;
			case DXGI_FORMAT_R32_UINT:			return FRAME_BUFFER_TEXTURE_FORMAT_R32UI;
			case DXGI_FORMAT_R16G16_UINT:		return FRAME_BUFFER_TEXTURE_FORMAT_RG16UI;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:	return FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
			case DXGI_FORMAT_D32_FLOAT:			return FRAME_BUFFER_TEXTURE_FORMAT_DEPTH;
			default:
				break;
			}
			return FrameBufferTextureFormat();
		}

		DXGI_FORMAT ShaderInputDataTypeToDXGIFormat(ShaderInputDataType type)
		{
			switch (type)
			{
			case SHADER_INPUT_DATA_TYPE_FLOAT1: return DXGI_FORMAT_R32_FLOAT;
			case SHADER_INPUT_DATA_TYPE_FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
			case SHADER_INPUT_DATA_TYPE_FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
			case SHADER_INPUT_DATA_TYPE_FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case SHADER_INPUT_DATA_TYPE_INT1: return DXGI_FORMAT_R32_SINT;
			case SHADER_INPUT_DATA_TYPE_INT2: return DXGI_FORMAT_R32G32_SINT;
			case SHADER_INPUT_DATA_TYPE_INT3: return DXGI_FORMAT_R32G32B32_SINT;
			case SHADER_INPUT_DATA_TYPE_INT4: return DXGI_FORMAT_R32G32B32A32_SINT;
			case SHADER_INPUT_DATA_TYPE_UINT1: return DXGI_FORMAT_R32_UINT;
			case SHADER_INPUT_DATA_TYPE_UINT2: return DXGI_FORMAT_R32G32_UINT;
			case SHADER_INPUT_DATA_TYPE_UINT3: return DXGI_FORMAT_R32G32B32_UINT;
			case SHADER_INPUT_DATA_TYPE_UINT4: return DXGI_FORMAT_R32G32B32A32_UINT;
			default:
				GE_CORE_ASSERT(false, "Unknown ShaderInputDataType!");
				break;
			}
			return DXGI_FORMAT();
		}
		DXGI_FORMAT RenderImage2DFormatToDXGIFormat(RenderImage2DFormat format)
		{
			switch (format)
			{
			case RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case RENDER_IMAGE_2D_FORMAT_RGBA8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
			case RENDER_IMAGE_2D_FORMAT_RGB8_UNORM: GE_CORE_ASSERT(false, "dx12 unsupport!");
			case RENDER_IMAGE_2D_FORMAT_RGB8_SNORM: GE_CORE_ASSERT(false, "dx12 unsupport!");
			default:
				GE_CORE_ASSERT(false, "Unknown RenderImage2DFormat!");
				break;
			}
			return DXGI_FORMAT();
		}

		D3D12_CULL_MODE CullModeToD3D12CullMode(CullMode mode)
		{
			switch (mode)
			{
			case CULL_MODE_NONE: return D3D12_CULL_MODE_NONE;
			case CULL_MODE_FRONT: return D3D12_CULL_MODE_FRONT;
			case CULL_MODE_BACK: return D3D12_CULL_MODE_BACK;
			default: GE_CORE_ASSERT(false, "Unknown CullMode!");
				break;
			}
			return D3D12_CULL_MODE();
		}

		D3D12_BLEND BlendFactorToD3D12Blend(BlendFactor factor)
		{
			switch (factor)
			{
			case BLEND_FACTOR_SRC_ALPHA: return D3D12_BLEND_SRC_ALPHA;
			case BLEND_FACTOR_DST_ALPHA: return D3D12_BLEND_DEST_ALPHA;
			case BLEND_FACTOR_SRC_COLOR: return D3D12_BLEND_SRC_COLOR;
			case BLEND_FACTOR_DST_COLOR: return D3D12_BLEND_DEST_COLOR;
			case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: return D3D12_BLEND_INV_SRC_ALPHA;
			case BLEND_FACTOR_ONE_MINUS_DST_ALPHA: return D3D12_BLEND_INV_DEST_ALPHA;
			case BLEND_FACTOR_ONE_MINUS_SRC_COLOR: return D3D12_BLEND_INV_SRC_COLOR;
			case BLEND_FACTOR_ONE_MINUS_DST_COLOR: return D3D12_BLEND_INV_DEST_COLOR;
			case BLEND_FACTOR_ONE: return D3D12_BLEND_ONE;
			case BLEND_FACTOR_ZERO: return D3D12_BLEND_ZERO;
			default:GE_CORE_ASSERT(false, "Unknown BlendFactor!");
				break;
			}
			return D3D12_BLEND();
		}

		D3D12_BLEND_OP BlendModeToD3D12BlendOp(BlendMode mode)
		{
			switch (mode)
			{
			case BLEND_MODE_NONE:				return D3D12_BLEND_OP_ADD;
			case BLEND_MODE_ADD:				return D3D12_BLEND_OP_ADD;
			case BLEND_MODE_SUBSTRACT:			return D3D12_BLEND_OP_SUBTRACT;
			case BLEND_MODE_REVERSE_SUBSTRACT:	return D3D12_BLEND_OP_REV_SUBTRACT;
			case BLEND_MODE_MIN:				return D3D12_BLEND_OP_MIN;
			case BLEND_MODE_MAX:				return D3D12_BLEND_OP_MAX;
			default:
				GE_CORE_ASSERT(false, "Unknown BlendMode!");
				return D3D12_BLEND_OP_ADD;
			}
		}

		D3D12_COMPARISON_FUNC CompareOperationToD3D12ComparisonFunc(CompareOperation op)
		{
			switch (op)
			{
			case COMPARE_OP_LESS:			return D3D12_COMPARISON_FUNC_LESS;
			case COMPARE_OP_GREATER:		return D3D12_COMPARISON_FUNC_GREATER;
			case COMPARE_OP_LESS_EQUAL:		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case COMPARE_OP_GREATER_EQUAL:	return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case COMPARE_OP_EQUAL:			return D3D12_COMPARISON_FUNC_EQUAL;
			case COMPARE_OP_NOT_EQUAL:		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case COMPARE_OP_ALWAYS:			return D3D12_COMPARISON_FUNC_ALWAYS;
			default:
				GE_CORE_ASSERT(false, "Unknown CompareOperation!");
				return D3D12_COMPARISON_FUNC_LESS;
			}
		}

		D3D12_PRIMITIVE_TOPOLOGY VertexTopologyToD3D12PrimitiveTopology(VertexTopology type)
		{
			switch (type)
			{
			case VERTEX_TOPOLOGY_POINT:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case VERTEX_TOPOLOGY_LINE:		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case VERTEX_TOPOLOGY_TRIANGLE:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			default:
				GE_CORE_ASSERT(false, "Unknown VertexTopology!");
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			}
		}
	}
}

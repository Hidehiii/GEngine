#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Core/Buffer.h"

#define SHADER_STAGE_VERTEX "vertex"
#define SHADER_STAGE_HULL "hull"
#define SHADER_STAGE_DOMAIN "domain"
#define SHADER_STAGE_GEOMETRY "geometry"
#define SHADER_STAGE_FRAGMENT "fragment"

#define SHADER_STAGE_AMPLIFICATION "amplification"
#define SHADER_STAGE_MESH "mesh"

#define SHADER_STAGE_COMPUTE "compute"

namespace GEngine
{
	enum Graphics_API
	{
		GRAPHICS_API_NONE = 0,
		GRAPHICS_API_OPENGL = 1,
		GRAPHICS_API_VULKAN = 2,
		GRAPHICS_API_DIRECT3DX12 = 3
	};

	struct GraphicsSpecification
	{
		Graphics_API		API;
		uint8_t				FramesInFlight;
		uint32_t			CommandBufferCount;
		uint32_t			DynamicUniformCount;
		uint32_t			ViewportWidth;
		uint32_t			ViewportHeight;
	};

	enum RenderImage2DFormat
	{
		RENDER_IMAGE_2D_FORMAT_NONE,
		RENDER_IMAGE_2D_FORMAT_RGBA8F,
		RENDER_IMAGE_2D_FORMAT_RGB8F,
	};

	enum ComputeImage2DFormat
	{
		COMPUTE_IMAGE_2D_FORMAT_NONE,
		COMPUTE_IMAGE_2D_FORMAT_RGBA32F,
		COMPUTE_IMAGE_2D_FORMAT_RGBA16F,
		COMPUTE_IMAGE_2D_FORMAT_RG32F,
		COMPUTE_IMAGE_2D_FORMAT_RG16F,
		COMPUTE_IMAGE_2D_FORMAT_R32F,
		COMPUTE_IMAGE_2D_FORMAT_R16F,
		COMPUTE_IMAGE_2D_FORMAT_RGBA32I,
		COMPUTE_IMAGE_2D_FORMAT_RGBA16I,
		COMPUTE_IMAGE_2D_FORMAT_RGBA8I,
		COMPUTE_IMAGE_2D_FORMAT_RGBA32UI,
		COMPUTE_IMAGE_2D_FORMAT_RGBA16UI,
		COMPUTE_IMAGE_2D_FORMAT_RGBA8UI,
		COMPUTE_IMAGE_2D_FORMAT_R32UI
	};

	enum SamplerFilter
	{
		SAMPLER_FILTER_LINEAR,
		SAMPLER_FILTER_NEAREST,
	};

	enum SamplerAddressMode
	{
		SAMPLER_ADDRESS_REPEAT,
		SAMPLER_ADDRESS_MIRRORED_REPEAT,
		SAMPLER_ADDRESS_CLAMP_TO_EDGE,
		SAMPLER_ADDRESS_CLAMP_TO_BORDER,
		SAMPLER_ADDRESS_MIRROR_CLAMP_TO_EDGE,
	};

	struct SamplerSpecification
	{
		SamplerFilter MagFilter		= SAMPLER_FILTER_LINEAR;
		SamplerFilter MinFilter		= SAMPLER_FILTER_LINEAR;
		SamplerFilter MipmapFilter	= SAMPLER_FILTER_LINEAR;

		SamplerAddressMode WrapU = SAMPLER_ADDRESS_REPEAT;
		SamplerAddressMode WrapV = SAMPLER_ADDRESS_REPEAT;
		SamplerAddressMode WrapW = SAMPLER_ADDRESS_REPEAT;

		bool operator==(const SamplerSpecification& other) const
		{
			return MagFilter == other.MagFilter &&
				MinFilter == other.MinFilter &&
				MipmapFilter == other.MipmapFilter &&
				WrapU == other.WrapU &&
				WrapV == other.WrapV &&
				WrapW == other.WrapW;
		}
	};

	enum BlendFactor
	{
		BLEND_FACTOR_SRC_ALPHA				= 1,
		BLEND_FACTOR_DST_ALPHA				= 2,
		BLEND_FACTOR_SRC_COLOR				= 3,
		BLEND_FACTOR_DST_COLOR				= 4,
		BLEND_FACTOR_ONE_MINUS_SRC_ALPHA	= 5,
		BLEND_FACTOR_ONE_MINUS_DST_ALPHA	= 6,
		BLEND_FACTOR_ONE_MINUS_SRC_COLOR	= 7,
		BLEND_FACTOR_ONE_MINUS_DST_COLOR	= 8,
		BLEND_FACTOR_ONE					= 9,
		BLEND_FACTOR_ZERO					= 10
	};

	enum CubeMapFace
	{
		CUBE_MAP_FACE_RIGHT		= 0, // +X
		CUBE_MAP_FACE_LEFT		= 1, // -X
		CUBE_MAP_FACE_TOP		= 2, // +Y
		CUBE_MAP_FACE_BUTTOM	= 3, // -Y
		CUBE_MAP_FACE_BACK		= 4, // +Z
		CUBE_MAP_FACE_FRONT		= 5  // -Z
	};

	enum FrameBufferTextureFormat
	{
		FRAME_BUFFER_TEXTURE_FORMAT_NONE = 0,

		// Color
		FRAME_BUFFER_TEXTURE_FORMAT_RGBA8,
		FRAME_BUFFER_TEXTURE_FORMAT_R32F,
		FRAME_BUFFER_TEXTURE_FORMAT_RG16F,
		FRAME_BUFFER_TEXTURE_FORMAT_R32I,
		FRAME_BUFFER_TEXTURE_FORMAT_RG16I,
		FRAME_BUFFER_TEXTURE_FORMAT_R32UI,
		FRAME_BUFFER_TEXTURE_FORMAT_RG16UI,

		// Depth,Stencil
		FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8,

		// Depth
		FRAME_BUFFER_TEXTURE_FORMAT_DEPTH
	};

	enum RenderPassBeginOperation
	{
		RENDER_PASS_BEGINE_OP_NONE = 0,  // begin
		RENDER_PASS_BEGINE_OP_LOAD = 1,  // begin
		RENDER_PASS_BEGINE_OP_CLEAR = 2,
	};

	enum RenderPassEndOperation
	{
		RENDER_PASS_END_OP_NONE = 0,
		RENDER_PASS_END_OP_STORE = 1,
	};

	enum CommandBufferType
	{
		COMMAND_BUFFER_TYPE_NONE,
		COMMAND_BUFFER_TYPE_GRAPHICS,
		COMMAND_BUFFER_TYPE_COMPUTE,
		COMMAND_BUFFER_TYPE_TRANSFER,
	};

	// Frame buffer texture specification
	// Contains the texture format
	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format)
			: TextureFormat(format) {
		}

		FrameBufferTextureFormat TextureFormat = FRAME_BUFFER_TEXTURE_FORMAT_NONE;

		bool operator==(const FrameBufferTextureSpecification& other) const { return TextureFormat == other.TextureFormat; }
	};

	// Frame buffer attachment specification
// Contains the frame buffer texture specification
// Contains the frame buffer texture format
	struct FrameBufferRenderTargetSpecification
	{
		FrameBufferRenderTargetSpecification() = default;
		FrameBufferRenderTargetSpecification(std::initializer_list<FrameBufferTextureSpecification> attachments)
			: Attachments(attachments) {
		}
		std::vector<FrameBufferTextureSpecification> Attachments;

		/*bool operator==(const FrameBufferAttachmentSpecification& other) const { return Attachments == other.Attachments; }*/
	};

	struct RenderPassOperation
	{
		RenderPassBeginOperation	ColorBegin			= RENDER_PASS_BEGINE_OP_CLEAR;
		RenderPassEndOperation		ColorEnd			= RENDER_PASS_END_OP_STORE;
		RenderPassBeginOperation	DepthStencilBegin	= RENDER_PASS_BEGINE_OP_CLEAR;
		RenderPassEndOperation		DepthStencilEnd		= RENDER_PASS_END_OP_STORE;

		bool operator==(const RenderPassOperation& other) const
		{
			return ColorBegin == other.ColorBegin &&
				ColorEnd == other.ColorEnd &&
				DepthStencilBegin == other.DepthStencilBegin &&
				DepthStencilEnd == other.DepthStencilEnd;
		}

		bool operator!=(const RenderPassOperation& other) const
		{
			return !this->operator==(other);
		}
	};

	struct RenderPassSpecification
	{
		std::vector<FrameBufferTextureSpecification>	ColorRTs;
		FrameBufferTextureSpecification					DepthStencil;
		int												Samples = 1;
		RenderPassOperation								Operation;

		bool operator==(const RenderPassSpecification& other) const
		{
			return ColorRTs == other.ColorRTs &&
				DepthStencil == other.DepthStencil &&
				Samples == other.Samples &&
				Operation == other.Operation;
		}
	};

	struct FrameBufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		std::vector<FrameBufferTextureSpecification>	ColorRTs;
		FrameBufferTextureSpecification					DepthStencil;
		uint32_t Samples = 1;
	};

	enum ShaderInputDataType
	{
		SHADER_INPUT_DATA_TYPE_NONE,

		SHADER_INPUT_DATA_TYPE_FLOAT1,
		SHADER_INPUT_DATA_TYPE_FLOAT2,
		SHADER_INPUT_DATA_TYPE_FLOAT3,
		SHADER_INPUT_DATA_TYPE_FLOAT4,

		SHADER_INPUT_DATA_TYPE_INT1,
		SHADER_INPUT_DATA_TYPE_INT2,
		SHADER_INPUT_DATA_TYPE_INT3,
		SHADER_INPUT_DATA_TYPE_INT4,
	};

	enum ShaderPropertyType
	{
		SHADER_PROPERTY_TYPE_NONE,
		SHADER_PROPERTY_TYPE_INT,
		SHADER_PROPERTY_TYPE_FLOAT,
		SHADER_PROPERTY_TYPE_VECTOR,
		SHADER_PROPERTY_TYPE_COLOR,

		SHADER_PROPERTY_TYPE_SAMPLER_2D,
		SHADER_PROPERTY_TYPE_SAMPLER_CUBE,
		SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY,

		SHADER_PROPERTY_TYPE_SAMPLER,
		SHADER_PROPERTY_TYPE_TEXTURE_2D,
		SHADER_PROPERTY_TYPE_TEXTURE_CUBE,
		SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY,

		SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D,
		SHADER_PROPERTY_TYPE_STORAGE_BUFFER,
	};

	enum VertexTopology
	{
		VERTEX_TOPOLOGY_POINT		= 0,
		VERTEX_TOPOLOGY_LINE		= 1,
		VERTEX_TOPOLOGY_TRIANGLE	= 2
	};

	enum CullMode
	{
		CULL_MODE_NONE = 0,
		CULL_MODE_FRONT = 1,
		CULL_MODE_BACK = 2
	};

	enum BlendMode
	{
		BLEND_MODE_NONE = 0,
		BLEND_MODE_ADD = 1,
		BLEND_MODE_SUBSTRACT = 2,
		BLEND_MODE_REVERSE_SUBSTRACT = 3,
		BLEND_MODE_MIN = 4,
		BLEND_MODE_MAX = 5,
	};

	enum CompareOperation
	{
		COMPARE_OP_LESS = 0,
		COMPARE_OP_GREATER = 1,
		COMPARE_OP_LESS_EQUAL = 2,
		COMPARE_OP_GREATER_EQUAL = 3,
		COMPARE_OP_EQUAL = 4,
		COMPARE_OP_NOT_EQUAL = 5,
		COMPARE_OP_ALWAYS = 6,
	};

	enum ColorMaskChannel
	{
		COLOR_MASK_CHANNLE_R = BIT(0),
		COLOR_MASK_CHANNLE_G = BIT(1),
		COLOR_MASK_CHANNLE_B = BIT(2),
		COLOR_MASK_CHANNLE_A = BIT(3),
	};

	struct RenderState
	{
		bool				DepthWrite = true;
		CompareOperation	DepthTestOp = COMPARE_OP_LESS_EQUAL;
		int					ColorMask = COLOR_MASK_CHANNLE_R | COLOR_MASK_CHANNLE_G | COLOR_MASK_CHANNLE_B | COLOR_MASK_CHANNLE_A;
		BlendFactor			BlendColorSrc = BLEND_FACTOR_ONE;
		BlendFactor			BlendColorDst = BLEND_FACTOR_ZERO;
		BlendFactor			BlendAlphaSrc = BLEND_FACTOR_ONE;
		BlendFactor			BlendAlphaDst = BLEND_FACTOR_ZERO;
		BlendMode			BlendColor = BLEND_MODE_NONE;
		BlendMode			BlendAlpha = BLEND_MODE_NONE;
		CullMode			Cull = CULL_MODE_BACK;
		std::string			Tag = "Default";
	};

	struct ConstShaderProperty
	{
		uint32_t			Size = 0;
		uint32_t			Location = 0;
	};

	struct ReferenceShaderProperty
	{
		uint32_t			Location = 0;
		void* Ptr = nullptr;
	};

	struct ShaderPass
	{
		RenderState													State;
		std::unordered_map<std::string, ConstShaderProperty>		ConstPropertiesDesc; // name : property
		Buffer														ConstProperties;
		std::unordered_map<std::string, ReferenceShaderProperty>	ReferenceProperties; // name : property
	};

	struct ShaderReflectionConstantInfo
	{
		std::string		Name;
		uint32_t		Size;
		uint32_t 		Location;
		uint32_t        Count;
	};

	struct ShaderReflectionInfo
	{
		std::vector<ShaderReflectionConstantInfo>		Constants;
	};
}
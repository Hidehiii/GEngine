#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
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

	// Frame buffer texture specification
	// Contains the texture format
	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format)
			: TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat = FRAME_BUFFER_TEXTURE_FORMAT_NONE;

		bool operator==(const FrameBufferTextureSpecification& other) const { return TextureFormat == other.TextureFormat; }
	};

	// Frame buffer attachment specification
	// Contains the frame buffer texture specification
	// Contains the frame buffer texture format
	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(std::initializer_list<FrameBufferTextureSpecification> attachments)
			: Attachments(attachments) {}
		std::vector<FrameBufferTextureSpecification> Attachments;

		/*bool operator==(const FrameBufferAttachmentSpecification& other) const { return Attachments == other.Attachments; }*/
	};

	enum RenderPassBeginOperation
	{
		RENDER_PASS_BEGINE_OP_NONE	= 0,  // begin
		RENDER_PASS_BEGINE_OP_LOAD	= 1,  // begin
		RENDER_PASS_BEGINE_OP_CLEAR = 2,
	};

	enum RenderPassEndOperation
	{
		RENDER_PASS_END_OP_NONE		= 0,
		RENDER_PASS_END_OP_STORE	= 1,
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

	

	class GENGINE_API RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification GetSpecification() { return m_Specification; }

		static Ref<RenderPass> GetRenderPass(const RenderPassSpecification& spec);

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
		
	protected:
		RenderPassSpecification		m_Specification;
	protected:
		static std::vector<Ref<RenderPass>>		s_RenderPasses;
	};
}



#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum class FrameBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		R32F,
		RG16F,
		R32I,
		RG16I,
		R32UI,
		RG16UI,

		// Depth,Stencil
		DEPTH24STENCIL8,

		// Depth
		DEPTH
	};

	// Frame buffer texture specification
	// Contains the texture format
	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format)
			: TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;

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

	enum class RenderPassBeginOperation
	{
		None = 0,  // begin
		Load = 1,  // begin
		Clear = 2,
	};

	enum class RenderPassEndOperation
	{
		None = 0,
		Store = 1,
	};

	struct RenderPassOperation
	{
		RenderPassBeginOperation	ColorBegin			= RenderPassBeginOperation::Clear;
		RenderPassEndOperation		ColorEnd			= RenderPassEndOperation::Store;
		RenderPassBeginOperation	DepthStencilBegin	= RenderPassBeginOperation::Clear;
		RenderPassEndOperation		DepthStencilEnd		= RenderPassEndOperation::Store;

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
		FrameBufferTextureSpecification					DepthStencilRT;
		int												Samples = 1;
		RenderPassOperation								Operation;

		bool operator==(const RenderPassSpecification& other) const
		{
			return ColorRTs == other.ColorRTs &&
				DepthStencilRT == other.DepthStencilRT &&
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



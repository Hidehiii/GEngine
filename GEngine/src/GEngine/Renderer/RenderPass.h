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

	enum class FrameBufferAttachmentsAction
	{
		None = 0,  // begin, end
		Load = 1,  // begin
		Clear = 2,  // end
		Store = 3   // end
	};

	struct RenderPassSpecification
	{
		std::vector<FrameBufferTextureSpecification>	ColorAttachments;
		FrameBufferTextureSpecification					DepthAttachment;
		int												Samples = 1;
		FrameBufferAttachmentsAction					AttachmentsBeginAction	= FrameBufferAttachmentsAction::Clear;
		FrameBufferAttachmentsAction					AttachmentsEndAction	= FrameBufferAttachmentsAction::Store;

		bool operator==(const RenderPassSpecification& other) const
		{
			return ColorAttachments == other.ColorAttachments &&
				DepthAttachment == other.DepthAttachment &&
				Samples == other.Samples &&
				AttachmentsBeginAction == other.AttachmentsBeginAction &&
				AttachmentsEndAction == other.AttachmentsEndAction;
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



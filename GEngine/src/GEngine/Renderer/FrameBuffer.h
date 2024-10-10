#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	enum class FrameBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth,Stencil
		DEPTH24STENCIL8,

		// Defaults
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
	};

	struct FrameBufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FrameBufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class GENGINE_API FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;
		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Resize(Vector2 size) = 0;

		virtual uint32_t GetColorAttachment(uint32_t index = 0) const = 0;
		virtual uint32_t GetDepthAttachment() const = 0;
		virtual int ReadPixelInt(int attachmentIndex, int x, int y) = 0;
		virtual void ClearAttachmentInt(int attachmentIndex, int val) = 0;

		virtual Vector2 GetSize() const { return { (float)GetSpecification().Width, (float)GetSpecification().Height }; }
		virtual float GetWidth() const { return (float)GetSpecification().Width; }
		virtual float GetHeight() const { return (float)GetSpecification().Height; }

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
		static FrameBuffer* GetCurrentFrameBuffer() { return s_CurrentFrameBuffer; }
	protected:
		static FrameBuffer*								s_CurrentFrameBuffer;

	protected:
		FrameBufferSpecification						m_Specification;
		FrameBufferTextureSpecification					m_DepthAttachmentSpec = FrameBufferTextureFormat::None;
		std::vector<FrameBufferTextureSpecification>	m_ColorAttachmentsSpecs;
	};

	namespace Utils
	{
		static bool isDepthFormat(FrameBufferTextureFormat format)
		{
			switch (format)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
				return true;
			case FrameBufferTextureFormat::DEPTH:
				return true;
			default:
				return false;
			}
			return false;
		}
	}
}



#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/Texture.h"

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

		virtual Vector2 GetSize() const { return { (float)GetSpecification().Width, (float)GetSpecification().Height }; }
		virtual float GetWidth() const { return (float)GetSpecification().Width; }
		virtual float GetHeight() const { return (float)GetSpecification().Height; }
		virtual int GetAttachmentCount() = 0;
		virtual Ref<Texture2D> GetColorAttachment(int index) = 0;
		virtual Ref<Texture2D> GetDepthAttachment() = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
		static Ref<FrameBuffer>	Recreate(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		static Ref<FrameBuffer>	Recreate(const Ref<FrameBuffer>& buffer, Vector2 size);
		operator bool() const { return this != nullptr; }

	protected:
		FrameBufferSpecification						m_Specification;
		FrameBufferTextureSpecification					m_DepthAttachmentSpec = FrameBufferTextureFormat::None;
		std::vector<FrameBufferTextureSpecification>	m_ColorAttachmentsSpecs;
	protected:
		static Vector2	GetCurrentFrameBufferSize() { return s_CurrentFrameBufferSize; }
		static Vector2									s_CurrentFrameBufferSize;

		friend class Renderer;
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



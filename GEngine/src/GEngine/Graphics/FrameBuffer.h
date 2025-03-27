#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "RenderPass.h"
#include "GEngine/Graphics/Texture.h"

namespace GEngine
{

	struct FrameBufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		std::vector<FrameBufferTextureSpecification>	ColorAttachments;
		FrameBufferTextureSpecification					DepthAttachment;
		uint32_t Samples = 1;
	};

	class CommandBuffer;

	class GENGINE_API FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;
		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		virtual void Begin(CommandBuffer* cmdBuffer) = 0;
		virtual void End(CommandBuffer* cmdBuffer) = 0;

		virtual void SetRenderPassOperation(const RenderPassOperation& op) = 0;

		virtual Vector2							GetSize() const { return { (float)GetSpecification().Width, (float)GetSpecification().Height }; }
		virtual float							GetWidth() const { return (float)GetSpecification().Width; }
		virtual float							GetHeight() const { return (float)GetSpecification().Height; }
		virtual uint32_t						GetSamples() const { return GetSpecification().Samples; }
		virtual int								GetRTCount() = 0;
		virtual int								GetColorRTCount() = 0;
		virtual Ref<Texture2D>					GetColorRT(int index) = 0;
		virtual Ref<Texture2D>					GetDepthStencilRT() = 0;
		virtual Ref<RenderPass>					GetRenderPass() = 0;

		static Ref<FrameBuffer> Create(const Ref<RenderPass>& renderPass, const Vector2& size);
		static Ref<FrameBuffer> Create(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height);
		static Ref<FrameBuffer>	Create(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		static Ref<FrameBuffer>	Create(const Ref<FrameBuffer>& buffer, Vector2 size);
		operator bool() const { return this != nullptr; }

	protected:
		FrameBufferSpecification						m_Specification;

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



#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer() override;

		

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Resize(Vector2 size) override;

		virtual int ReadPixelInt(int attachmentIndex, int x, int y) override;
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual void ClearAttachmentInt(int attachmentIndex, int val) override;
		virtual uint32_t GetColorAttachment(uint32_t index = 0) const override { GE_CORE_ASSERT(index < m_ColorAttachments.size(), "Out of range"); return m_ColorAttachments[index]; }
		virtual uint32_t GetDepthAttachment() const override { return m_DepthAttachment; }

	private:
		void CreateBuffer();
	private:
		uint32_t										m_RendererID = 0;
		uint32_t										m_DepthAttachment = 0;
		std::vector<uint32_t>							m_ColorAttachments;
	};
}



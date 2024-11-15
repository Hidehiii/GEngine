#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "OpenGLTexture2D.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		OpenGLFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		virtual ~OpenGLFrameBuffer() override;

		

		virtual void Begin() override;
		virtual void End() override;
		virtual int GetAttachmentCount() override { return m_DepthAttachment == 0 ? m_ColorAttachments.size() : m_ColorAttachments.size() + 1; };

		virtual int ReadPixelInt(int attachmentIndex, int x, int y) override;
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual void ClearAttachmentInt(int attachmentIndex, int val) override;
		virtual Ref<Texture2D> GetColorAttachment(int index) override;
		virtual Ref<Texture2D> GetDepthAttachment() override;

	private:
		void CreateBuffer();
	private:
		uint32_t										m_RendererID = 0;
		uint32_t										m_DepthAttachment = 0;
		std::vector<uint32_t>							m_ColorAttachments;
		std::vector <Ref<OpenGLTexture2D>>				m_ColorAttachmentsTexture2D;
		Ref<OpenGLTexture2D>							m_DepthAttachmentTexture2D;
	};
}



#include "GEpch.h"
#include "OpenGLFrameBuffer.h"
#include "OpenGLUtils.h"
#include "GEngine/Graphics/Graphics.h"

#include <glad/glad.h>
namespace GEngine
{
    namespace Utils
    {
        

        static void CreateTextures(bool multisample, uint32_t* outID, uint32_t count)
        {
			glCreateTextures(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, count, outID);
        }

        static void BindTexture(bool multisample, uint32_t id)
        {
            glBindTexture(multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, id);
        }
        static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, uint32_t index)
        {
            if (samples > 1)
            {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D_MULTISAMPLE, id, 0);
			}
            else
            {
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, id, 0);
			}
            
		}
        static void AttachDepthTexture(uint32_t id, int samples, GLenum internalFormat, GLenum attachmentType, uint32_t width, uint32_t height)
        {
            if (samples > 1)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D_MULTISAMPLE, id, 0);
            }
            else
            {
                glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, id, 0);
            }
            
        }
    }

    OpenGLFrameBuffer::OpenGLFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
    {
        m_Specification.ColorRTs                = renderPass->GetSpecification().ColorRTs;
        m_Specification.DepthStencil            = renderPass->GetSpecification().DepthStencil;
        m_Specification.Samples                 = renderPass->GetSpecification().Samples;
        m_Specification.Width                   = width;
        m_Specification.Height                  = height;

        m_RenderPass = std::dynamic_pointer_cast<OpenGLRenderPass>(renderPass);

        CreateBuffer();
    }
    OpenGLFrameBuffer::OpenGLFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
    {
        m_Specification         = buffer->GetSpecification();
        m_Specification.Width   = width;
        m_Specification.Height  = height;

        m_RenderPass = std::dynamic_pointer_cast<OpenGLRenderPass>(buffer->GetRenderPass());

        CreateBuffer();
    }
    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_FrameBuffer);
        if (m_Specification.Samples > 1)
        {
            glDeleteFramebuffers(1, &m_MultiSampleFrameBuffer);
            glDeleteTextures(m_MultiSampleColorAttachments.size(), m_MultiSampleColorAttachments.data());
            glDeleteTextures(1, &m_MultiSampleDepthStencilAttachment);
        }
    }
    void OpenGLFrameBuffer::CreateBuffer()
    {

        if (m_Specification.Samples > 1)
        {
            glCreateFramebuffers(1, &m_MultiSampleFrameBuffer);

            glBindFramebuffer(GL_FRAMEBUFFER, m_MultiSampleFrameBuffer);

            // Attachments
            if (m_Specification.ColorRTs.size())
            {
                m_MultiSampleColorAttachments.resize(m_Specification.ColorRTs.size());
                Utils::CreateTextures(m_Specification.Samples > 1, m_MultiSampleColorAttachments.data(), m_MultiSampleColorAttachments.size());

                for (size_t i = 0; i < m_MultiSampleColorAttachments.size(); i++)
                {
                    Utils::BindTexture(m_Specification.Samples > 1, m_MultiSampleColorAttachments[i]);
					Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], 1, Utils::FrameBufferTextureFormatToGLInternalFormat(m_Specification.ColorRTs[i].TextureFormat),
						Utils::FrameBufferTextureFormatToGLDataFormat(m_Specification.ColorRTs[i].TextureFormat), m_Specification.Width, m_Specification.Height, i);
                }
            }

            if (m_Specification.DepthStencil.TextureFormat != FrameBufferTextureFormat::None)
            {
                Utils::CreateTextures(m_Specification.Samples > 1, &m_MultiSampleDepthStencilAttachment, 1);
                Utils::BindTexture(m_Specification.Samples > 1, m_MultiSampleDepthStencilAttachment);
                switch (m_Specification.DepthStencil.TextureFormat)
                {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                {
                    Utils::AttachDepthTexture(m_MultiSampleDepthStencilAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                }
                case FrameBufferTextureFormat::DEPTH:
                {
                    Utils::AttachDepthTexture(m_MultiSampleDepthStencilAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                }
                }
            }

            if (m_MultiSampleColorAttachments.size() >= 1)
            {
                GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                                       GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};
                glDrawBuffers(m_MultiSampleColorAttachments.size(), buffers);
            }
            else if (m_MultiSampleColorAttachments.empty())
            {
                glDrawBuffers(GL_NONE, GL_NONE);
            }


            GE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "multisample Framebuffer is incomplete!");

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glCreateFramebuffers(1, &m_FrameBuffer);
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

        // Attachments
        if (m_Specification.ColorRTs.size())
        {
            m_ColorAttachments.resize(m_Specification.ColorRTs.size());
            Utils::CreateTextures(false, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
            {
                Utils::BindTexture(false, m_ColorAttachments[i]);
                Utils::AttachColorTexture(m_ColorAttachments[i], 1, Utils::FrameBufferTextureFormatToGLInternalFormat(m_Specification.ColorRTs[i].TextureFormat), 
                    Utils::FrameBufferTextureFormatToGLDataFormat(m_Specification.ColorRTs[i].TextureFormat), m_Specification.Width, m_Specification.Height, i);

                Ref<OpenGLTexture2D> texture = CreateRef<OpenGLTexture2D>(m_ColorAttachments[i]);
                m_ColorRTs.push_back(texture);
            }
        }

        if (m_Specification.DepthStencil.TextureFormat != FrameBufferTextureFormat::None)
        {
            Utils::CreateTextures(false, &m_DepthStencilAttachment, 1);
            Utils::BindTexture(false, m_DepthStencilAttachment);
            switch (m_Specification.DepthStencil.TextureFormat)
            {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                {
                    Utils::AttachDepthTexture(m_DepthStencilAttachment, 1, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				    break;
				}
                case FrameBufferTextureFormat::DEPTH:
                {
                    Utils::AttachDepthTexture(m_DepthStencilAttachment, 1, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                }
            }
            m_DepthStencil = CreateRef<OpenGLTexture2D>(m_DepthStencilAttachment);
        }


        if (m_ColorAttachments.size() >= 1)
        {
            GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                                   GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};
            glDrawBuffers(m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.empty())
        {
            glDrawBuffers(GL_NONE, GL_NONE);
        }

        
        GE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void OpenGLFrameBuffer::Begin(CommandBuffer* cmdBuffer)
    {
		if (m_Specification.Samples > 1)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_MultiSampleFrameBuffer);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
		}

		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
		glDisable(GL_FRAMEBUFFER_SRGB);
		glDepthMask(GL_TRUE);
        glDepthRange(Graphics::IsReverseDepth() ? 1 : 0, Graphics::IsReverseDepth() ? 0 : 1);
        glClearDepth(Graphics::IsReverseDepth() ? 0 : 1);
		if (m_RenderPass->GetSpecification().Operation.ColorBegin == RenderPassBeginOperation::Clear)
		{
			glClear(GL_COLOR_BUFFER_BIT);
		}
		if (m_RenderPass->GetSpecification().Operation.DepthStencilBegin == RenderPassBeginOperation::Clear)
		{
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
    }
    void OpenGLFrameBuffer::End(CommandBuffer* cmdBuffer)
    {
        if (m_Specification.Samples > 1)
        {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MultiSampleFrameBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
			glBlitFramebuffer(0, 0, m_Specification.Width, m_Specification.Height, 0, 0, m_Specification.Width, m_Specification.Height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void OpenGLFrameBuffer::SetRenderPassOperation(const RenderPassOperation& op)
    {
        auto spec       = m_RenderPass->GetSpecification();
        spec.Operation  = op;
        m_RenderPass    = std::dynamic_pointer_cast<OpenGLRenderPass>(RenderPass::Create(spec));
    }
    Ref<Texture2D> OpenGLFrameBuffer::GetColorRT(int index)
    {
        GE_CORE_ASSERT(index < m_ColorAttachments.size(), "index out of range");
        return m_ColorRTs.at(index);
    }
    Ref<Texture2D> OpenGLFrameBuffer::GetDepthStencil()
    {
        GE_CORE_ASSERT(m_DepthStencilAttachment != 0, "No depth frame buffer");
        return m_DepthStencil;
    }
}
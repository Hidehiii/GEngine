#include "GEpch.h"
#include "OpenGLFrameBuffer.h"

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

    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec)
    {
        m_Specification = spec;
        for (auto format : m_Specification.Attachments.Attachments)
        {
            if (Utils::isDepthFormat(format.TextureFormat))
            {
                m_DepthAttachmentSpec = format;
            }
            else
            {
                m_ColorAttachmentsSpecs.emplace_back(format);
            }
		}

        CreateBuffer();
    }
    OpenGLFrameBuffer::OpenGLFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
    {
        m_Specification         = buffer->GetSpecification();
        m_Specification.Width   = width;
        m_Specification.Height  = height;
        for (auto format : m_Specification.Attachments.Attachments)
        {
            if (Utils::isDepthFormat(format.TextureFormat))
            {
                m_DepthAttachmentSpec = format;
            }
            else
            {
                m_ColorAttachmentsSpecs.emplace_back(format);
            }
        }

        CreateBuffer();
    }
    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        /*glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);*/
        if (m_Specification.Samples > 1)
        {
            glDeleteFramebuffers(1, &m_MultiSampleRendererID);
            glDeleteTextures(m_MultiSampleColorAttachments.size(), m_MultiSampleColorAttachments.data());
            glDeleteTextures(1, &m_MultiSampleDepthAttachment);
        }
    }
    void OpenGLFrameBuffer::CreateBuffer()
    {

        if (m_Specification.Samples > 1)
        {
            glCreateFramebuffers(1, &m_MultiSampleRendererID);

            glBindFramebuffer(GL_FRAMEBUFFER, m_MultiSampleRendererID);

            // Attachments
            if (m_ColorAttachmentsSpecs.size())
            {
                m_MultiSampleColorAttachments.resize(m_ColorAttachmentsSpecs.size());
                Utils::CreateTextures(m_Specification.Samples > 1, m_MultiSampleColorAttachments.data(), m_MultiSampleColorAttachments.size());

                for (size_t i = 0; i < m_MultiSampleColorAttachments.size(); i++)
                {
                    Utils::BindTexture(m_Specification.Samples > 1, m_MultiSampleColorAttachments[i]);
                    switch (m_ColorAttachmentsSpecs[i].TextureFormat)
                    {
                    case FrameBufferTextureFormat::RGBA8:
                    {
                        Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
                        break;
                    }
                    case FrameBufferTextureFormat::R32F:
                    {
                        Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_R32F, GL_R, m_Specification.Width, m_Specification.Height, i);
                        break;
                    }
                    case FrameBufferTextureFormat::RG16F:
                    {
						Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_RG16F, GL_RG, m_Specification.Width, m_Specification.Height, i);
						break;
                    }
					case FrameBufferTextureFormat::R32I:
					{
						Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_R32I, GL_R, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::RG16I:
					{
						Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_RG16I, GL_RG, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::R32UI:
					{
						Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_R32UI, GL_R, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::RG16UI:
					{
						Utils::AttachColorTexture(m_MultiSampleColorAttachments[i], m_Specification.Samples, GL_RG16UI, GL_RG, m_Specification.Width, m_Specification.Height, i);
						break;
					}
                    }
                }
            }

            if (m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None)
            {
                Utils::CreateTextures(m_Specification.Samples > 1, &m_MultiSampleDepthAttachment, 1);
                Utils::BindTexture(m_Specification.Samples > 1, m_MultiSampleDepthAttachment);
                switch (m_DepthAttachmentSpec.TextureFormat)
                {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                {
                    Utils::AttachDepthTexture(m_MultiSampleDepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                }
                case FrameBufferTextureFormat::DEPTH:
                {
                    Utils::AttachDepthTexture(m_MultiSampleDepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
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

        glCreateFramebuffers(1, &m_RendererID);
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        // Attachments
        if (m_ColorAttachmentsSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentsSpecs.size());
            Utils::CreateTextures(false, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
            {
                Utils::BindTexture(false, m_ColorAttachments[i]);
                switch (m_ColorAttachmentsSpecs[i].TextureFormat)
                {
                    case FrameBufferTextureFormat::RGBA8:
                    {
                        Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
                        break;
                    }
                    case FrameBufferTextureFormat::R32F:
                    {
                        Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_R32F, GL_R, m_Specification.Width, m_Specification.Height, i);
                        break;
                    }
					case FrameBufferTextureFormat::RG16F:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_RG16F, GL_RG, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::R32I:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_R32I, GL_R, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::RG16I:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_RG16I, GL_RG, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::R32UI:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_R32UI, GL_R, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FrameBufferTextureFormat::RG16UI:
					{
						Utils::AttachColorTexture(m_ColorAttachments[i], 1, GL_RG16UI, GL_RG, m_Specification.Width, m_Specification.Height, i);
						break;
					}
                }
                Ref<OpenGLTexture2D> texture = CreateRef<OpenGLTexture2D>(m_ColorAttachments[i], false);
                m_ColorAttachmentsTexture2D.push_back(texture);
            }
        }

        if (m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None)
        {
            Utils::CreateTextures(false, &m_DepthAttachment, 1);
            Utils::BindTexture(false, m_DepthAttachment);
            switch (m_DepthAttachmentSpec.TextureFormat)
            {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                {
                    Utils::AttachDepthTexture(m_DepthAttachment, 1, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				    break;
				}
                case FrameBufferTextureFormat::DEPTH:
                {
                    Utils::AttachDepthTexture(m_DepthAttachment, 1, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                }
            }
            m_DepthAttachmentTexture2D = CreateRef<OpenGLTexture2D>(m_DepthAttachment, false);
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
    void OpenGLFrameBuffer::Begin()
    {
        if (m_Specification.Samples > 1)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_MultiSampleRendererID);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        }
        
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        s_CurrentFrameBufferSize = Vector2(m_Specification.Width, m_Specification.Height);
    }
    void OpenGLFrameBuffer::End()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    Ref<Texture2D> OpenGLFrameBuffer::GetColorAttachment(int index)
    {
        GE_CORE_ASSERT(index < m_ColorAttachments.size(), "index out of range");
        if (m_Specification.Samples > 1)
        {
			GLint currentFbo;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MultiSampleRendererID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RendererID);
            glBlitFramebuffer(0, 0, m_Specification.Width, m_Specification.Height, 0, 0, m_Specification.Width, m_Specification.Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
        }
        return m_ColorAttachmentsTexture2D.at(index);
    }
    Ref<Texture2D> OpenGLFrameBuffer::GetDepthAttachment()
    {
        GE_CORE_ASSERT(m_DepthAttachment != 0, "No depth frame buffer");
        if (m_Specification.Samples > 1)
        {
			GLint currentFbo;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MultiSampleRendererID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RendererID);
            glBlitFramebuffer(0, 0, m_Specification.Width, m_Specification.Height, 0, 0, m_Specification.Width, m_Specification.Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
        }
        return m_DepthAttachmentTexture2D;
    }
}
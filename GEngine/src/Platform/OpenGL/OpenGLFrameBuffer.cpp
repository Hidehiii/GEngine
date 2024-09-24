#include "GEpch.h"
#include "OpenGLFrameBuffer.h"
#include <glad/glad.h>
namespace GEngine
{
    static uint32_t s_MaxFrameBufferSize = 8192;

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
        static GLenum FrameBufferTextureFormatToGLFormat(FrameBufferTextureFormat format)
        {
            switch (format)
            {
                case FrameBufferTextureFormat::RGBA8:
					return GL_RGBA8;
                case FrameBufferTextureFormat::RED_INTEGER:
                    return GL_RED_INTEGER;
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                    return GL_DEPTH24_STENCIL8;
                case FrameBufferTextureFormat::DEPTH:
                    return GL_DEPTH_COMPONENT;
            }
            GE_CORE_ASSERT(false, "Unknown FrameBufferTextureFormat");
            return GL_NONE;
        }
    }

    OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec)
        : m_Specification(spec)
    {
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

        Invalidate();
    }
    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    }
    void OpenGLFrameBuffer::Invalidate()
    {
        if (m_RendererID != 0)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            m_ColorAttachments.clear();
            m_DepthAttachment = 0;
        }

        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        // Attachments
        if (m_ColorAttachmentsSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentsSpecs.size());
            Utils::CreateTextures(m_Specification.Samples > 1, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
            {
                Utils::BindTexture(m_Specification.Samples > 1, m_ColorAttachments[i]);
                switch (m_ColorAttachmentsSpecs[i].TextureFormat)
                {
                    case FrameBufferTextureFormat::RGBA8:
                    {
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
                        break;
                    }
                    case FrameBufferTextureFormat::RED_INTEGER:
                    {
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
                        break;
                    }
                }
            }
        }

        if (m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None)
        {
            Utils::CreateTextures(m_Specification.Samples > 1, &m_DepthAttachment, 1);
            Utils::BindTexture(m_Specification.Samples > 1, m_DepthAttachment);
            switch (m_DepthAttachmentSpec.TextureFormat)
            {
                case FrameBufferTextureFormat::DEPTH24STENCIL8:
                {
                    Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				    break;
				}
                case FrameBufferTextureFormat::DEPTH:
                {
                    Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                }
            }
        }

        if (m_ColorAttachments.size() >= 1)
        {
            GE_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Out of range");
            GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            glDrawBuffers(m_ColorAttachments.size(), buffers);
        }
        else if(m_ColorAttachments.empty())
        {
            glDrawBuffers(GL_NONE, GL_NONE);
        }

        
        GE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void OpenGLFrameBuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
    }
    void OpenGLFrameBuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width <= 0 || height <= 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize)
        {
			GE_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}

        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }
    void OpenGLFrameBuffer::Resize(Vector2 size)
    {
        Resize((uint32_t)size.value.x, (uint32_t)size.value.y);
    }
    int OpenGLFrameBuffer::ReadPixelInt(int attachmentIndex, int x, int y)
    {
        GE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Out of range");
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        int pixel = -1;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel);
        return pixel;
    }
    void OpenGLFrameBuffer::ClearAttachmentInt(int attachmentIndex, int val)
    {
        GE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Out of range");

        glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::FrameBufferTextureFormatToGLFormat(m_ColorAttachmentsSpecs[attachmentIndex].TextureFormat), GL_INT, &val);
    }
}
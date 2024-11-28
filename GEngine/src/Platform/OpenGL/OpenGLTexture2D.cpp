#include "GEpch.h"
#include "OpenGLTexture2D.h"

#include "stb_image.h"
#include <glad/glad.h>
#include "OpenGLUtils.h"

namespace GEngine
{
	


	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		GE_PROFILE_FUNCTION();

		int				width, height, channels;
		stbi_uc*		data;
		stbi_set_flip_vertically_on_load(1);
		{
			GE_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path)")
			data		= stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width			= width;
		m_Height		= height;
		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat	= GL_RGBA8;
			dataFormat		= GL_RGBA;
			m_Format = Image2DFormat::R8G8B8A8F;
		}
		else if (channels == 3)
		{
			internalFormat	= GL_RGB8;
			dataFormat		= GL_RGB;
			m_Format = Image2DFormat::R8G8B8F;
		}

		GE_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, Image2DFormat format)
		: m_Width(width), m_Height(height)
	{
		GE_PROFILE_FUNCTION();

		m_Format = format;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, Utils::Image2DFormatToGLInternalFormat(format), m_Width, m_Height);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, Image2DFormat format)
		: m_Width(width), m_Height(height)
	{
		GE_PROFILE_FUNCTION();
		m_Format = format;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, Utils::Image2DFormatToGLInternalFormat(format), m_Width, m_Height);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		SetData(data, size);
	}
	OpenGLTexture2D::OpenGLTexture2D(uint32_t rendererID, bool isMultiSample)
	{
		m_RendererID = rendererID;
		m_MultiSample = isMultiSample;
	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		GE_PROFILE_FUNCTION();

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, Utils::Image2DFormatToGLDataFormat(m_Format), GL_UNSIGNED_BYTE, data);
	}
	void OpenGLTexture2D::Bind(const uint32_t slot)
	{	
		GE_PROFILE_FUNCTION();
		GE_CORE_ASSERT(m_MultiSample == false, "Can not bind a multisample texture!");
		glBindTextureUnit(slot, m_RendererID);
	}
	void OpenGLTexture2D::Unbind()
	{
	}
}
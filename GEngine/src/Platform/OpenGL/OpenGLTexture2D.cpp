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
		int				width, height, channels;
		stbi_uc*		data;
		stbi_set_flip_vertically_on_load(1);
		data		= stbi_load(path.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width			= width;
		m_Height		= height;
		if (channels == 4)
		{
			m_Format = RenderImage2DFormat::RGBA8F;
		}
		else if (channels == 3)
		{
			m_Format = RenderImage2DFormat::RGB8F;
		}
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Texture2D);
		glTextureStorage2D(m_Texture2D, m_MipLevels, Utils::RenderImage2DFormatToGLInternalFormat(m_Format), m_Width, m_Height);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		SetData(data, m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format));
		stbi_image_free(data);
	}
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Texture2D);
		glTextureStorage2D(m_Texture2D, m_MipLevels, Utils::RenderImage2DFormatToGLInternalFormat(format), m_Width, m_Height);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Texture2D);
		glTextureStorage2D(m_Texture2D, m_MipLevels, Utils::RenderImage2DFormatToGLInternalFormat(format), m_Width, m_Height);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Texture2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		SetData(data, size);
	}
	OpenGLTexture2D::OpenGLTexture2D(uint32_t rendererID)
	{
		m_Texture2D = rendererID;
	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_Texture2D);
	}
	void OpenGLTexture2D::SetData(const void* data, uint32_t size)
	{
		glTextureSubImage2D(m_Texture2D, 0, 0, 0, m_Width, m_Height, Utils::RenderImage2DFormatToGLDataFormat(m_Format), GL_UNSIGNED_BYTE, data);
		if (m_GenerateMipmap)
		{
			glGenerateTextureMipmap(m_Texture2D);
		}
		
	}
	void OpenGLTexture2D::Bind(const uint32_t slot)
	{	
		glBindTextureUnit(slot, m_Texture2D);
	}

}
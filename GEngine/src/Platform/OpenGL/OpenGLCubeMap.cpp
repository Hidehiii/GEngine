#include "GEpch.h"
#include "OpenGLCubeMap.h"
#include "stb_image.h"
#include <glad/glad.h>
#include "OpenGLUtils.h"
#include "OpenGLTexture2D.h"

namespace GEngine
{
	OpenGLCubeMap::OpenGLCubeMap(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		m_Format = format;
		m_Width = width;
		m_Height = height;
		m_GenerateMipmap = generateMipmap;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_CubeMap);
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		glTextureStorage2D(m_CubeMap, m_MipLevels, Utils::RenderImage2DFormatToGLInternalFormat(m_Format), m_Width, m_Height);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	OpenGLCubeMap::OpenGLCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
	{
		m_Path.push_back(rightPath);
		m_Path.push_back(leftPath);
		m_Path.push_back(topPath);
		m_Path.push_back(buttomPath);
		m_Path.push_back(backPath);
		m_Path.push_back(frontPath);
		m_GenerateMipmap = generateMipmap;
		int	width, height, channels;
		stbi_uc* data;
		data = stbi_load(rightPath.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load image!");
		m_Width	= width;
		m_Height = height;
		if (m_GenerateMipmap)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
		}
		if (channels == 4)
		{
			m_Format = RENDER_IMAGE_2D_FORMAT_RGBA8F;
		}
		else if (channels == 3)
		{
			m_Format = RENDER_IMAGE_2D_FORMAT_RGB8F;
		}
		stbi_image_free(data);
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_CubeMap);
		glTextureStorage2D(m_CubeMap, m_MipLevels, Utils::RenderImage2DFormatToGLInternalFormat(m_Format), m_Width, m_Height);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_CubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		LoadImageData();
	}
	OpenGLCubeMap::~OpenGLCubeMap()
	{
		glDeleteTextures(1, &m_CubeMap);
	}
	void OpenGLCubeMap::SetData(const void* data, uint32_t size)
	{
		uint32_t tempSize = m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format);
		for (int i = 0; i < 6 / tempSize; i++)
		{
			uint32_t offset = tempSize * i;
			void* tempData = new std::byte[tempSize];
			memcpy(tempData, ((std::byte*)data) + offset, tempSize);
			SetData(tempData, tempSize, (CubeMapFace)i);
			delete[](std::byte*)tempData;
		}
	}
	void OpenGLCubeMap::SetData(const void* data, uint32_t size, CubeMapFace face)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (uint16_t)face, 0, 0, 0, m_Width, m_Height, Utils::RenderImage2DFormatToGLDataFormat(m_Format), GL_UNSIGNED_BYTE, data);
		if (m_GenerateMipmap)
		{
			glGenerateTextureMipmap(m_CubeMap);
		}
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	void OpenGLCubeMap::SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face)
	{
		GLint currentFbo;
		GLuint fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
		glCreateFramebuffers(1, &fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetOpenGLID(), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap);
		glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (uint16_t)face, 0, 0, 0, 0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
		glDeleteFramebuffers(1, &fbo);
		if (m_GenerateMipmap)
		{
			glGenerateTextureMipmap(m_CubeMap);
		}
		
	}
	void OpenGLCubeMap::Bind(CommandBuffer* cmdBuffer, const uint32_t slot)
	{
		glBindTextureUnit(slot, m_CubeMap);
	}
	std::string OpenGLCubeMap::GetPath() const
	{
		std::string path;
		for (auto str : m_Path)
		{
			path += "(" + str + ")";
		}
		return path;
	}
	void OpenGLCubeMap::LoadImageData()
	{
		for (int i = 0; i < m_Path.size(); i++)
		{
			int	width, height, channels;
			stbi_uc* data;
			stbi_set_flip_vertically_on_load(1);
			data = stbi_load(m_Path.at(i).c_str(), &width, &height, &channels, 0);
			GE_CORE_ASSERT(data, "Failed to load image!");
			GE_CORE_ASSERT(m_Width == width, " The cube map image is not the same size!");
			GE_CORE_ASSERT(m_Height = height, " The cube map image is not the same size!");
			RenderImage2DFormat format = RENDER_IMAGE_2D_FORMAT_NONE;
			if (channels == 4)
			{
				format = RENDER_IMAGE_2D_FORMAT_RGBA8F;
			}
			else if (channels == 3)
			{
				format = RENDER_IMAGE_2D_FORMAT_RGB8F;
			}
			GE_CORE_ASSERT(m_Format == format, " The cube map image is not the same format!");

			SetData(data, m_Width * m_Height * RenderImage2DFormatChannelSize(m_Format), (CubeMapFace)i);
			stbi_image_free(data);
		}
	}
}


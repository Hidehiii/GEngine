#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Texture.h"
#include <glad/glad.h>

namespace GEngine
{
	class GENGINE_API OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		OpenGLTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		OpenGLTexture2D(uint32_t rendererID);
		virtual ~OpenGLTexture2D();

		virtual uint32_t	GetWidth() const override { return m_Width; }
		virtual uint32_t	GetHeight() const override { return m_Height; }
		virtual void		SetData(const void* data, uint32_t size) override;
		virtual void		Bind(const uint32_t slot = 0) override;
		virtual std::string GetPath() const override { return m_Path; };

		uint32_t			GetOpenGLID() const { return m_Texture2D; }

		virtual bool operator==(const Texture& other) const override { return m_Texture2D == ((OpenGLTexture2D&)other).m_Texture2D; };

	private:
		std::string				m_Path;
		uint32_t				m_Width, m_Height;
		uint32_t				m_Texture2D;
		RenderImage2DFormat		m_Format;
	};
}



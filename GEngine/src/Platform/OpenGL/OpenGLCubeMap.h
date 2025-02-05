#pragma once
#include "GEngine/Renderer/Texture.h"

namespace GEngine
{
	class GENGINE_API OpenGLCubeMap : public CubeMap
	{
	public:
		OpenGLCubeMap(uint32_t width, uint32_t height, bool generateMipmap = false, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		OpenGLCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap);
		virtual ~OpenGLCubeMap();

		virtual uint32_t	GetWidth() const override { return m_Width; }
		virtual uint32_t	GetHeight() const override { return m_Height; }
		virtual std::string GetPath() const override;

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const void* data, uint32_t size, CubeMapFace face) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face) override;
		virtual void Bind(const uint32_t slot = 0) override;
		
		uint32_t GetRendererID() const { return m_RendererID; }

		virtual bool operator==(const Texture& other) const override { return m_RendererID == ((OpenGLCubeMap&)other).m_RendererID; };
	private:
		void LoadImageData();
	private:
		std::vector<std::string>		m_Path;
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height;
		RenderImage2DFormat		m_Format;
	};
}



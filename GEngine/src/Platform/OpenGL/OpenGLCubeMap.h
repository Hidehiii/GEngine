#pragma once
#include "GEngine/Renderer/Texture.h"

namespace GEngine
{
	class GENGINE_API OpenGLCubeMap : public CubeMap
	{
	public:
		OpenGLCubeMap(uint32_t width, uint32_t height, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		OpenGLCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath);
		virtual ~OpenGLCubeMap();
	private:
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height;
		RenderImage2DFormat		m_Format;
	};
}



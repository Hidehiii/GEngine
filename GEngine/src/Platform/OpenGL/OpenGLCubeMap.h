#pragma once
#include "GEngine/Graphics/Texture.h"

namespace GEngine
{
	class GENGINE_API OpenGLCubeMap : public CubeMap
	{
	public:
		OpenGLCubeMap(uint32_t width, uint32_t height, bool generateMipmap = false, RenderImage2DFormat format = RENDER_IMAGE_2D_FORMAT_RGBA8F);
		OpenGLCubeMap(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap);
		virtual ~OpenGLCubeMap();

		virtual std::string GetPath() const override;

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const void* data, uint32_t size, CubeMapFace face) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face) override;
		virtual void Bind(CommandBuffer* cmdBuffer, const uint32_t slot = 0) override;
		
		uint32_t GetOpenGLID() const { return m_CubeMap; }

		virtual bool operator==(const CubeMap& other) const override { return m_CubeMap == ((OpenGLCubeMap&)other).m_CubeMap; };
	private:
		void LoadImageData();
	private:
		std::vector<std::string>		m_Path;
		uint32_t m_CubeMap;
	};
}



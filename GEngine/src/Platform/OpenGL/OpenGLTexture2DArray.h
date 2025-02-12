#pragma once
#include "GEngine/Renderer/Texture.h"

namespace GEngine
{
	class GENGINE_API OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format);
		virtual ~OpenGLTexture2DArray() override;

		virtual void Bind(const uint32_t slot = 0) override;

		virtual void SetData(const void* data, uint32_t size) override;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, uint32_t layer) override;
		virtual void SetData(const void* data, uint32_t size, uint32_t layer) override;
		
		virtual std::string GetPath() const override { return "NULL"; }

		virtual bool operator==(const Texture2DArray& other) const override { return m_TextureArray == ((OpenGLTexture2DArray&)other).m_TextureArray; };
 	private:
		uint32_t	m_TextureArray;
	};
}



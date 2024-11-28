#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/ImageFormat.h"


namespace GEngine
{
	class GENGINE_API Texture
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void Bind(const uint32_t slot = 0) = 0;
		virtual void Unbind() = 0;
		virtual std::string GetPath() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	

	class GENGINE_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, Texture2DFormat format = Texture2DFormat::RGBA);
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, void* data, uint32_t size, Texture2DFormat format = Texture2DFormat::RGBA);

		// 默认白色纹理
		static Ref<Texture2D> WhiteTexture();
	private:
		static Ref<Texture2D>	s_WhiteTexture2D;
	};
}



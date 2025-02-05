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
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void Bind(const uint32_t slot = 0) = 0;
		virtual std::string GetPath() const = 0;
		virtual uint32_t GetMipLevels() { return m_MipLevels; }
		virtual bool IsGenerateMipmap() { return m_GenerateMipmap; }

		virtual bool operator==(const Texture& other) const = 0;

	protected:
		bool m_GenerateMipmap = true;
		uint32_t m_MipLevels = 1;
	};

	

	class GENGINE_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);

		// 默认白色纹理
		static Ref<Texture2D> White();

	private:
		static Ref<Texture2D>	s_WhiteTexture2D;
	};

	class GENGINE_API CubeMap : public Texture
	{
	public:
		enum class CubeMapFace
		{
			Right	= 0, // +X
			Left	= 1, // -X
			Top		= 2, // +Y
			Buttom	= 3, // -Y
			Back	= 4, // +Z
			Front	= 5  // -Z
		};
		virtual void SetData(const void* data, uint32_t size, CubeMapFace face) = 0;
		virtual void SetData(const Ref<Texture2D>& texture, uint32_t width, uint32_t height, CubeMapFace face) = 0;

		static Ref<CubeMap> Create(uint32_t width, uint32_t height, bool generateMipmap = false, RenderImage2DFormat format = RenderImage2DFormat::RGBA8F);
		static Ref<CubeMap> Create(const std::string& rightPath, const std::string& leftPath,
			const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap = false);

		// 默认白色
		static Ref<CubeMap> White();
	private:
		static Ref<CubeMap> s_WhiteCubeMap;
	};
}



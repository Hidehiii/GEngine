#pragma once

#include "GEngine/Renderer/Texture.h"

namespace GEngine
{
	class GENGINE_API VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path);
		VulkanTexture2D(uint32_t width, uint32_t height);
		virtual ~VulkanTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Bind(const uint32_t slot = 0) const override;
		virtual std::string GetPath() const override { return m_Path; };

		// VK的相等不是比较ID，而是比较是否是同一个对象（晚点改）
		virtual bool operator==(const Texture& other) const override { return m_RendererID == ((VulkanTexture2D&)other).m_RendererID; };

	private:
		std::string			m_Path;
		uint32_t			m_Width, m_Height;
		uint32_t			m_RendererID;
	};
}



#include "GEpch.h"
#include "Texture.h"
#include "Graphics.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/OpenGL/OpenGLCubeMap.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/OpenGL/OpenGLTexture2DArray.h"
#include "Platform/Vulkan/VulkanTexture2DArray.h"

namespace GEngine
{
	Ref<Texture2D>	Texture2D::s_WhiteTexture2D = nullptr;
	Ref<CubeMap> CubeMap::s_WhiteCubeMap = nullptr;

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLTexture2D>(width, height, format);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			return CreateRef<VulkanTexture2D>(width, height, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLTexture2D>(path);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			return CreateRef<VulkanTexture2D>(path);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLTexture2D>(width, height, data, size, format);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			return CreateRef<VulkanTexture2D>(width, height, data, size, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::White()
	{
		if (s_WhiteTexture2D == nullptr)
		{
			uint32_t		whiteTexture2DData = 0xffffffff;
			s_WhiteTexture2D = Texture2D::Create(1, 1, &whiteTexture2DData, sizeof(uint32_t));
		}
		return s_WhiteTexture2D;
	}

	Ref<CubeMap> CubeMap::Create(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLCubeMap>(width, height, generateMipmap, format);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			return CreateRef<VulkanCubeMap>(width, height, generateMipmap, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<CubeMap> CubeMap::Create(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLCubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			return CreateRef<VulkanCubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<CubeMap> CubeMap::White()
	{
		if (s_WhiteCubeMap == nullptr)
		{
			uint32_t whiteCubeMapData = 0xffffffff;
			s_WhiteCubeMap = CubeMap::Create(1, 1);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CubeMapFace::Right);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CubeMapFace::Left);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CubeMapFace::Top);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CubeMapFace::Buttom);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CubeMapFace::Back);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CubeMapFace::Front);
		}							
		return s_WhiteCubeMap;
	}
	Ref<Texture2DArray> Texture2DArray::Create(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLTexture2DArray>(width, height, layers, format);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			return CreateRef<VulkanTexture2DArray>(width, height, layers, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
}
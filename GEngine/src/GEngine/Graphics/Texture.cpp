#include "GEpch.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Graphics/Graphics.h"
#include "Platform/OpenGL/OpenGLCubeMap.h"
#include "Platform/Vulkan/VulkanCubeMap.h"
#include "Platform/D3D12/D3D12CubeMap.h"
#include "Platform/OpenGL/OpenGLTexture2DArray.h"
#include "Platform/Vulkan/VulkanTexture2DArray.h"
#include "Platform/D3D12/D3D12Texture2DArray.h"
#include "Platform/OpenGL/OpenGLTexture2DCombineSampler.h"
#include "Platform/Vulkan/VulkanTexture2DCombineSampler.h"
#include "Platform/OpenGL/OpenGLCubeMapCombineSampler.h"
#include "Platform/Vulkan/VulkanCubeMapCombineSampler.h"
#include "Platform/D3D12/D3D12TextureCombineSampler.h"

namespace GEngine
{
	Ref<Texture2D>	Texture2D::s_WhiteTexture2D = nullptr;
	std::unordered_map<std::string, Ref<Texture2D>> Texture2D::s_Texture2Ds;
	Ref<CubeMap> CubeMap::s_WhiteCubeMap = nullptr;
	std::unordered_map<std::string, Ref<CubeMap>> CubeMap::s_CubeMaps;

	Ref<Texture2DCombineSampler> Texture2DCombineSampler::Create(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
				GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
				return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLTexture2DCombineSampler>(texture, sampler);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanTexture2DCombineSampler>(texture, sampler);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			return CreateRef<D3D12Texture2DCombineSampler>(texture, sampler);
		}
		default:
			GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
			break;
		}
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		return Graphics::GetRenderDevice().CreateTexture2D(width, height, format);
	}
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		return Graphics::GetRenderDevice().CreateTexture2D(path);
	}
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
		return Graphics::GetRenderDevice().CreateTexture2D(width, height, data, size, format);
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

	void Texture2D::ShutdownCache()
	{
		s_Texture2Ds.clear();
		s_WhiteTexture2D.reset();
	}

	Ref<CubeMapCombineSampler> CubeMapCombineSampler::Create(const Ref<CubeMap>& cubemap, const Ref<Sampler>& sampler)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLCubeMapCombineSampler>(cubemap, sampler);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanCubeMapCombineSampler>(cubemap, sampler);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			return CreateRef<D3D12CubeMapCombineSampler>(cubemap, sampler);
		}
		default:
			GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
			break;
		}
		return nullptr;
	}

	Ref<CubeMap> CubeMap::Create(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLCubeMap>(width, height, generateMipmap, format);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanCubeMap>(width, height, generateMipmap, format);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			return CreateRef<D3D12CubeMap>(width, height, generateMipmap, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<CubeMap> CubeMap::Create(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLCubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanCubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			return CreateRef<D3D12CubeMap>(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
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
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CUBE_MAP_FACE_RIGHT);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CUBE_MAP_FACE_LEFT);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CUBE_MAP_FACE_TOP);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CUBE_MAP_FACE_BUTTOM);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CUBE_MAP_FACE_BACK);
			s_WhiteCubeMap->SetData(&whiteCubeMapData, sizeof(uint32_t), CUBE_MAP_FACE_FRONT);
		}							
		return s_WhiteCubeMap;
	}
	void CubeMap::ShutdownCache()
	{
		s_CubeMaps.clear();
		s_WhiteCubeMap.reset();
	}
	Ref<Texture2DArray> Texture2DArray::Create(uint32_t width, uint32_t height, uint32_t layers, RenderImage2DFormat format)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLTexture2DArray>(width, height, layers, format);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanTexture2DArray>(width, height, layers, format);
		}
		case GRAPHICS_API_DIRECT3DX12: {
			return CreateRef<D3D12Texture2DArray>(width, height, layers, format);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
}

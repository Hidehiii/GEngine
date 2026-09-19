#include "GEpch.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	Ref<Texture2D>	Texture2D::s_WhiteTexture2D = nullptr;
	std::unordered_map<std::string, Ref<Texture2D>> Texture2D::s_Texture2Ds;
	Ref<CubeMap> CubeMap::s_WhiteCubeMap = nullptr;
	std::unordered_map<std::string, Ref<CubeMap>> CubeMap::s_CubeMaps;

	Ref<Texture2DCombineSampler> Texture2DCombineSampler::Create(const Ref<Texture2D>& texture, const Ref<Sampler>& sampler)
	{
		return Graphics::GetRenderDevice().CreateTexture2DCombineSampler(texture, sampler);
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
		return Graphics::GetRenderDevice().CreateCubeMapCombineSampler(cubemap, sampler);
	}

	Ref<CubeMap> CubeMap::Create(uint32_t width, uint32_t height, bool generateMipmap, RenderImage2DFormat format)
	{
		return Graphics::GetRenderDevice().CreateCubeMap(width, height, generateMipmap, format);
	}
	Ref<CubeMap> CubeMap::Create(const std::string& rightPath, const std::string& leftPath, const std::string& topPath, const std::string& buttomPath, const std::string& backPath, const std::string& frontPath, bool generateMipmap)
	{
		return Graphics::GetRenderDevice().CreateCubeMap(rightPath, leftPath, topPath, buttomPath, backPath, frontPath, generateMipmap);
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
		return Graphics::GetRenderDevice().CreateTexture2DArray(width, height, layers, format);
	}
}

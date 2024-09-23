#pragma once

#include "GEngine/Core/Core.h"
#include "RendererAPI.h"
#include "GEngine/Math/Math.h"



namespace GEngine
{
	class GENGINE_API RenderCommand
	{
	public:
		inline void static Init();
		inline static void SetClearColor(const Vector4& color);
		inline static void Clear();
		inline static void DrawTriangles(const Ref<VertexArray>& vertexArray, const uint32_t count);
		inline static void DrawTriangles(const Ref<VertexArray>& vertexArray);
		inline static void DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t count);
		inline static void DrawPoints(const Ref<VertexArray>& vertexArray, const uint32_t count);
		inline static void SetLineWidth(float width);
		inline static void SetPointSize(float size);
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		inline static float GetTime();
		inline static void SetCull(Material_CullMode mode);
		inline static void SetBlend(Material_BlendMode mode, uint32_t source, uint32_t dest);
		inline static void EnableDepthMask(bool enabled);
		inline static void EnableDepthTest(bool enabled);
		inline static void SetRendererAPI(RendererAPI::API api);
	private:
		inline static RendererAPI* s_RendererAPI;
	};
}




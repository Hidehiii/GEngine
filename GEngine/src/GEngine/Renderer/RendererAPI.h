#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/RenderBuffer.h"
#include "Material.h"

namespace GEngine
{
	class GENGINE_API RendererAPI
	{
	public:
		enum class API
		{
			None = 0, 
			OpenGL = 1,
			Vulkan = 2
		};

	public:
		virtual void Init() = 0;
		virtual void Uninit() = 0;

		virtual void SetClearColor(const Vector4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawTriangles(uint32_t indexCount = 0) = 0;
		virtual void DrawLines(uint32_t indexCount = 0) = 0;
		virtual void DrawPoints(uint32_t indexCount = 0) = 0;

		virtual void DrawTrianglesInstance(uint32_t indexCount, uint32_t instanceCount) = 0;

		virtual void SetLineWidth(float width) = 0;
		virtual void SetPointSize(float size) = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void EnableDepthWrite(bool enabled) = 0;
		virtual void SetDepthTest(CompareOperation op) = 0;
		virtual void SetCull(CullMode mode) = 0;
		virtual void SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) = 0;

		virtual float GetTime() = 0;

		virtual void BeginDrawCommand() = 0;
		virtual void EndDrawCommand() = 0;

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }
		inline static API Current() { return s_API; }
	private:
		inline static API s_API		= API::OpenGL;
	};

}




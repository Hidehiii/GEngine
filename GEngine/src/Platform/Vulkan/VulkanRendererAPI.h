#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RendererAPI.h"

#include <vulkan/vulkan.h>

namespace GEngine
{
	class GENGINE_API VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;

		virtual void SetClearColor(const Vector4& color) override;
		virtual void Clear() override;


		virtual void DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void SetDepthTest(bool enabled) override;
		virtual void SetBlend(bool enabled) override;
		virtual void SetBlendFunction(uint32_t source, uint32_t destination) override;


		virtual void DrawTrianglesInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) override;

		virtual void SetLineWidth(float width) override;
		virtual void SetPointSize(float size) override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual float GetTime() override;
	};
}



#pragma once
#include "GEngine/Core/Core.h"
#include "Material.h"
#include "RenderBuffer.h"
namespace GEngine
{
	class GENGINE_API Pipeline
	{
	public:
		virtual ~Pipeline() = default;
		virtual void Render(uint32_t instanceCount = 1, uint32_t indexCount = 0) = 0;
		virtual Ref<VertexBuffer> GetVertexBuffer() = 0;
		virtual void SetVertexBuffer(Ref<VertexBuffer>& buffer) = 0;
		virtual Ref<Material> GetMaterial() = 0;
		virtual void SetMaterial(Ref<Material>& material) = 0;
		static Ref<Pipeline> Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
	};
}




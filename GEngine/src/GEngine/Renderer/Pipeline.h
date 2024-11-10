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
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual Ref<VertexBuffer> GetVertexBuffer() = 0;

		virtual Ref<Material> GetMaterial() = 0;
		static Ref<Pipeline> Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
	};
}




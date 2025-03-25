#pragma once
#include "GEngine/Core/Core.h"
#include "Material.h"
#include "GraphicsBuffer.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
namespace GEngine
{
	class CommandBuffer;

	class GENGINE_API GraphicsPipeline
	{
	public:
		virtual ~GraphicsPipeline() = default;
		

		virtual Ref<VertexBuffer>	GetVertexBuffer() = 0;
		virtual void				SetVertexBuffer(Ref<VertexBuffer>& buffer) = 0;

		virtual Ref<Material>	GetMaterial() = 0;
		virtual void			SetMaterial(Ref<Material>& material) = 0;

		static Ref<GraphicsPipeline> Create(const Ref<Material>& material, const Ref<VertexBuffer>& vertexBuffer);
	protected:
		virtual void Render(CommandBuffer* cmdBuffer, const Ref<FrameBuffer>& frameBuffer, uint32_t instanceCount = 1, uint32_t indexCount = 0) = 0;

		friend class CommandBuffer;
	};
}




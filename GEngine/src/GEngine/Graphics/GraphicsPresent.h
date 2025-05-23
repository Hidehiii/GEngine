#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Math/Math.h"
#include <set>

namespace GEngine
{
	class CommandBuffer;
	class Scene;
	class GraphicsPipeline;
	class ComputePipeline;

	class GENGINE_API GraphicsPresent
	{
	public:
		GraphicsPresent() {}
		virtual bool AquireImage() { return true; }
		virtual void Begin() {}
		virtual void End() {}

		static Ref<CommandBuffer> GetCommandBuffer();
		static void Render(Ref<GraphicsPipeline>& pipeline, const std::string& pass, uint32_t instanceCount = 1, uint32_t indexCount = 0);
		static void Compute(Ref<ComputePipeline>& pipeline, const std::string& pass, uint32_t x, uint32_t y, uint32_t z);
	protected:
		friend class Application;

		static Scope<GraphicsPresent> Create();
	protected:
		static std::vector<Ref<CommandBuffer>>	s_CommandBuffers;
	};
}
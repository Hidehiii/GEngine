#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/RenderGraph.h"
#include "GEngine/Components/Camera/CameraComponent.h"

#include <string>
#include <vector>

namespace GEngine
{
	class CommandBuffer;
	class RenderPipeline;
	class RenderPipelineBuilder;
	class RenderPipelineContext;

	class GENGINE_API RenderPipelinePass
	{
	public:
		virtual ~RenderPipelinePass() = default;

		virtual const std::string& GetName() const = 0;
		virtual void Configure(RenderPipelineBuilder& builder) = 0;
		virtual void Execute(RenderPipelineContext& context) = 0;
		virtual void Present(RenderPipelineContext& context)
		{
			(void)context;
		}
	};

	class GENGINE_API RenderPipelineBuilder final
	{
	public:
		explicit RenderPipelineBuilder(RenderGraph& graph)
			: m_Graph(graph)
		{
		}

		RenderGraph::TargetHandle CreateRenderTarget(std::string name,
			const RenderGraph::AttachmentSpecification& specification, uint32_t width, uint32_t height)
		{
			return m_Graph.CreateRenderTarget(std::move(name), specification, width, height);
		}

		RenderGraph::PassBuilder AddGraphicsPass(std::string name, RenderGraph::TargetHandle target,
			RenderGraph::RecordCallback record)
		{
			return m_Graph.BuildGraphicsPass(std::move(name), target, std::move(record));
		}

		RenderGraph::PassBuilder AddComputePass(std::string name, RenderGraph::RecordCallback record)
		{
			return m_Graph.BuildComputePass(std::move(name), std::move(record));
		}

		RenderGraph::PassBuilder AddTransferCopyPass(std::string name, RenderGraph::RecordCallback record)
		{
			return m_Graph.BuildTransferCopyPass(std::move(name), std::move(record));
		}

		RenderGraph& GetGraph()
		{
			return m_Graph;
		}

	private:
		RenderGraph& m_Graph;
	};

	class GENGINE_API RenderPipelineContext final
	{
	public:
		RenderPipelineContext(RenderGraph& graph, const Ref<CommandBuffer>& command,
			std::vector<CameraComponent>& cameras, uint64_t frame)
			: Graph(graph), Command(command), Cameras(cameras), Frame(frame)
		{
		}

		RenderGraph& Graph;
		const Ref<CommandBuffer>& Command;
		std::vector<CameraComponent>& Cameras;
		uint64_t Frame;
	};

	class GENGINE_API RenderPipeline
	{
	public:
		RenderPipeline() = default;
		virtual ~RenderPipeline() = default;

		void AddPass(const Ref<RenderPipelinePass>& pass);
		void RemovePass(const std::string& name);
		void Execute(const Ref<CommandBuffer>& completion, std::vector<CameraComponent>& cameras);
		void Present(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras);
		void Reset();

		RenderGraph& GetGraph()
		{
			return m_Graph;
		}

		const RenderGraph& GetGraph() const
		{
			return m_Graph;
		}

	protected:
		virtual void Configure(RenderPipelineBuilder& builder)
		{
			(void)builder;
		}

	private:
		void ConfigureIfNeeded();

	private:
		RenderGraph m_Graph;
		std::vector<Ref<RenderPipelinePass>> m_Passes;
		uint64_t m_Frame = 0;
		bool m_Configured = false;
	};
}

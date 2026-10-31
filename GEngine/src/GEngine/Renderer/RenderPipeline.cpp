#include "GEpch.h"
#include "GEngine/Renderer/RenderPipeline.h"

#include <algorithm>
#include <stdexcept>

namespace GEngine
{
	void RenderPipeline::AddPass(const Ref<RenderPipelinePass>& pass)
	{
		if (!pass)
			throw std::invalid_argument("RenderPipeline requires a valid pass.");
		for (const auto& existing : m_Passes)
			if (existing->GetName() == pass->GetName())
				throw std::invalid_argument("RenderPipeline pass names must be unique: " + pass->GetName());
		m_Passes.push_back(pass);
		m_Configured = false;
	}

	void RenderPipeline::RemovePass(const std::string& name)
	{
		const auto iterator = std::remove_if(m_Passes.begin(), m_Passes.end(),
			[&name](const Ref<RenderPipelinePass>& pass)
			{
				return pass->GetName() == name;
			});
		if (iterator == m_Passes.end())
			throw std::invalid_argument("RenderPipeline pass does not exist: " + name);
		m_Passes.erase(iterator, m_Passes.end());
		m_Configured = false;
	}

	void RenderPipeline::Execute(const Ref<CommandBuffer>& completion, std::vector<CameraComponent>& cameras)
	{
		if (!completion)
			throw std::invalid_argument("RenderPipeline requires a completion command buffer.");
		ConfigureIfNeeded();
		RenderPipelineContext context(m_Graph, completion, cameras, m_Frame);
		for (const auto& pass : m_Passes)
			pass->Execute(context);
		m_Graph.ExecuteGpu(completion);
		++m_Frame;
	}

	void RenderPipeline::Present(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras)
	{
		if (!command)
			throw std::invalid_argument("RenderPipeline requires a presentation command buffer.");
		ConfigureIfNeeded();
		RenderPipelineContext context(m_Graph, command, cameras, m_Frame);
		for (const auto& pass : m_Passes)
			pass->Present(context);
	}

	void RenderPipeline::Reset()
	{
		m_Graph.Reset();
		m_Passes.clear();
		m_Configured = false;
	}

	void RenderPipeline::ConfigureIfNeeded()
	{
		if (m_Configured)
			return;
		RenderPipelineBuilder builder(m_Graph);
		Configure(builder);
		m_Configured = true;
	}
}

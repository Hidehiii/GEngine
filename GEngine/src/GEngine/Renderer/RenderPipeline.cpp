#include "GEpch.h"
#include "GEngine/Renderer/RenderPipeline.h"

#include <algorithm>
#include <stdexcept>

namespace GEngine
{
	void RenderPipeline::AddPass(const Ref<RenderPipelinePass>& pass)
	{
		if (m_State != State::Unconfigured && m_State != State::Configuring)
			throw std::logic_error("RenderPipeline requires Reset before pass mutation.");
		if (!pass)
			throw std::invalid_argument("RenderPipeline requires a valid pass.");
		for (const auto& existing : m_Passes)
			if (existing->GetName() == pass->GetName())
				throw std::invalid_argument("RenderPipeline pass names must be unique: " + pass->GetName());
		m_Passes.push_back(pass);
	}

	void RenderPipeline::RemovePass(const std::string& name)
	{
		if (m_State != State::Unconfigured && m_State != State::Configuring)
			throw std::logic_error("RenderPipeline requires Reset before pass mutation.");
		const auto iterator = std::remove_if(m_Passes.begin(), m_Passes.end(),
			[&name](const Ref<RenderPipelinePass>& pass)
			{
				return pass->GetName() == name;
			});
		if (iterator == m_Passes.end())
			throw std::invalid_argument("RenderPipeline pass does not exist: " + name);
		m_Passes.erase(iterator, m_Passes.end());
	}

	void RenderPipeline::Execute(const Ref<CommandBuffer>& completion, std::vector<CameraComponent>& cameras)
	{
		if (!completion)
			throw std::invalid_argument("RenderPipeline requires a completion command buffer.");
		ConfigureIfNeeded();
		if (m_State != State::Ready)
			throw std::logic_error("RenderPipeline Execute requires an idle configured pipeline.");
		m_State = State::Executing;
		RenderPipelineContext context(m_Graph, completion, cameras, m_Frame);
		try
		{
			for (const auto& pass : m_Passes)
				pass->Execute(context);
			m_Graph.ExecuteGpu(completion);
			m_State = State::AwaitingPresent;
		}
		catch (...)
		{
			m_State = State::Failed;
			throw;
		}
	}

	void RenderPipeline::Present(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras)
	{
		if (!command)
			throw std::invalid_argument("RenderPipeline requires a presentation command buffer.");
		if (m_State != State::AwaitingPresent)
			throw std::logic_error("RenderPipeline Present requires one successful Execute.");
		m_State = State::Presenting;
		RenderPipelineContext context(m_Graph, command, cameras, m_Frame);
		try
		{
			for (const auto& pass : m_Passes)
				pass->Present(context);
			++m_Frame;
			m_State = State::Ready;
		}
		catch (...)
		{
			m_State = State::Failed;
			throw;
		}
	}

	void RenderPipeline::Reset()
	{
		if (m_State != State::Unconfigured && m_State != State::Ready && m_State != State::Failed)
			throw std::logic_error("RenderPipeline Reset is only valid outside frame/configuration hooks.");
		m_Graph.Reset();
		m_Passes.clear();
		m_Frame = 0;
		m_State = State::Unconfigured;
	}

	void RenderPipeline::ConfigureIfNeeded()
	{
		if (m_State != State::Unconfigured)
			return;
		m_State = State::Configuring;
		try
		{
			RenderPipelineBuilder builder(m_Graph);
			Configure(builder);
			m_State = State::ConfiguringPasses;
			for (const auto& pass : m_Passes)
				pass->Configure(builder);
			m_State = State::Ready;
		}
		catch (...)
		{
			m_State = State::Failed;
			throw;
		}
	}
}

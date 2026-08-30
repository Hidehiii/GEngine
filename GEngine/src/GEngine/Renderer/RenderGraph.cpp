#include "GEpch.h"
#include "GEngine/Renderer/RenderGraph.h"
#include "GEngine/Compute/StorageBuffer.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Graphics/Texture.h"

namespace GEngine
{
	RenderGraph::PassHandle RenderGraph::AddPass(std::string name, ExecuteCallback execute)
	{
		GE_CORE_ASSERT(execute, "Render-graph pass requires an execute callback.");
		m_IsCompiled = false;
		m_Passes.push_back({ std::move(name), std::move(execute), {}, {}, {} });
		return static_cast<PassHandle>(m_Passes.size() - 1);
	}

	RenderGraph::PassHandle RenderGraph::AddPass(std::string name, std::function<void()> execute)
	{
		GE_CORE_ASSERT(execute, "Render-graph pass requires an execute callback.");
		return AddPass(std::move(name), [execute = std::move(execute)](FrameContext&) { execute(); });
	}

	void RenderGraph::AddDependency(PassHandle pass, PassHandle dependency)
	{
		GE_CORE_ASSERT(pass < m_Passes.size() && dependency < m_Passes.size(), "Render-graph dependency is invalid.");
		GE_CORE_ASSERT(pass != dependency, "A render-graph pass cannot depend on itself.");
		m_IsCompiled = false;
		const auto& dependencies = m_Passes[pass].Dependencies;
		if (std::find(dependencies.begin(), dependencies.end(), dependency) == dependencies.end())
			m_Passes[pass].Dependencies.push_back(dependency);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportResource(std::string name, ResourceState initialState)
	{
		GE_CORE_ASSERT(!name.empty(), "Render-graph resources require a name.");
		m_IsCompiled = false;
		m_Resources.push_back({ std::move(name), initialState, nullptr, GraphicsResourceType::Unknown });
		return static_cast<ResourceHandle>(m_Resources.size() - 1);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportExternalResource(std::string name, void* nativeResource, ResourceState initialState, GraphicsResourceType resourceType)
	{
		GE_CORE_ASSERT(nativeResource != nullptr, "External render-graph resources require a native resource.");
		const auto resource = ImportResource(std::move(name), initialState);
		m_Resources[resource].NativeResource = nativeResource;
		m_Resources[resource].Type = resourceType;
		return resource;
	}

	RenderGraph::ResourceHandle RenderGraph::ImportTexture(std::string name, const Ref<Texture>& texture, ResourceState initialState)
	{
		GE_CORE_ASSERT(texture, "Render-graph texture imports require a texture.");
		if (void* nativeResource = texture->GetNativeResource())
			return ImportExternalResource(std::move(name), nativeResource, initialState, GraphicsResourceType::Texture);
		return ImportResource(std::move(name), initialState);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportStorageBuffer(std::string name, const Ref<StorageBuffer>& buffer, ResourceState initialState)
	{
		GE_CORE_ASSERT(buffer, "Render-graph storage-buffer imports require a buffer.");
		if (void* nativeResource = buffer->GetNativeResource())
			return ImportExternalResource(std::move(name), nativeResource, initialState, GraphicsResourceType::Buffer);
		return ImportResource(std::move(name), initialState);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportStorageImage(std::string name, const Ref<StorageImage2D>& image, ResourceState initialState)
	{
		GE_CORE_ASSERT(image, "Render-graph storage-image imports require an image.");
		if (void* nativeResource = image->GetNativeResource())
			return ImportExternalResource(std::move(name), nativeResource, initialState, GraphicsResourceType::Texture);
		return ImportResource(std::move(name), initialState);
	}

	void RenderGraph::Read(PassHandle pass, ResourceHandle resource, ResourceState state)
	{
		AddAccess(pass, resource, state, false);
	}

	void RenderGraph::Write(PassHandle pass, ResourceHandle resource, ResourceState state)
	{
		AddAccess(pass, resource, state, true);
	}

	void RenderGraph::SetTransitionCallback(TransitionCallback callback)
	{
		m_TransitionCallback = std::move(callback);
	}

	void* RenderGraph::GetNativeResource(ResourceHandle resource) const
	{
		GE_CORE_ASSERT(resource < m_Resources.size(), "Render-graph resource is invalid.");
		return m_Resources[resource].NativeResource;
	}

	bool RenderGraph::Compile()
	{
		m_ExecutionOrder.clear();
		std::vector<uint8_t> states(m_Passes.size(), 0);
		for (PassHandle pass = 0; pass < m_Passes.size(); ++pass)
		{
			if (!Visit(pass, states))
			{
				m_ExecutionOrder.clear();
				m_IsCompiled = false;
				return false;
			}
		}

		BuildResourceTransitions();
		m_IsCompiled = true;
		return true;
	}

	void RenderGraph::Execute(FrameContext& frameContext)
	{
		GE_CORE_ASSERT(m_IsCompiled || Compile(), "Render graph contains a dependency cycle.");
		for (const PassHandle pass : m_ExecutionOrder)
		{
			for (const auto& transition : m_Passes[pass].Transitions)
			{
				if (m_TransitionCallback)
					m_TransitionCallback(frameContext, transition.Resource, m_Resources[transition.Resource].Type, transition.Before, transition.After);
			}
			m_Passes[pass].Execute(frameContext);
		}
	}

	void RenderGraph::Execute()
	{
		FrameContext frameContext;
		Execute(frameContext);
	}

	void RenderGraph::Reset()
	{
		m_Passes.clear();
		m_Resources.clear();
		m_ExecutionOrder.clear();
		m_IsCompiled = false;
	}

	void RenderGraph::AddResourceDependency(PassHandle pass, ResourceHandle resource, bool isWrite)
	{
		for (PassHandle previous = 0; previous < pass; ++previous)
		{
			for (const auto& access : m_Passes[previous].ResourceAccesses)
			{
				if (access.Resource == resource && (isWrite || access.IsWrite))
					AddDependency(pass, previous);
			}
		}
	}

	void RenderGraph::AddAccess(PassHandle pass, ResourceHandle resource, ResourceState state, bool isWrite)
	{
		GE_CORE_ASSERT(pass < m_Passes.size(), "Render-graph pass is invalid.");
		GE_CORE_ASSERT(resource < m_Resources.size(), "Render-graph resource is invalid.");
		GE_CORE_ASSERT(state != ResourceState::Undefined, "Render-graph accesses require a concrete resource state.");

		AddResourceDependency(pass, resource, isWrite);
		m_IsCompiled = false;
		m_Passes[pass].ResourceAccesses.push_back({ resource, state, isWrite });
	}

	void RenderGraph::BuildResourceTransitions()
	{
		std::vector<ResourceState> states;
		states.reserve(m_Resources.size());
		for (const auto& resource : m_Resources)
			states.push_back(resource.InitialState);

		for (const PassHandle pass : m_ExecutionOrder)
		{
			auto& transitions = m_Passes[pass].Transitions;
			transitions.clear();
			for (const auto& access : m_Passes[pass].ResourceAccesses)
			{
				auto& currentState = states[access.Resource];
				if (currentState != access.State)
				{
					transitions.push_back({ access.Resource, currentState, access.State });
					currentState = access.State;
				}
			}
		}
	}

	bool RenderGraph::Visit(PassHandle pass, std::vector<uint8_t>& states)
	{
		if (states[pass] == 2)
			return true;
		if (states[pass] == 1)
		{
			GE_CORE_ERROR("Render graph contains a cycle at pass '{0}'.", m_Passes[pass].Name);
			return false;
		}

		states[pass] = 1;
		for (const PassHandle dependency : m_Passes[pass].Dependencies)
		{
			if (!Visit(dependency, states))
				return false;
		}

		states[pass] = 2;
		m_ExecutionOrder.push_back(pass);
		return true;
	}
}

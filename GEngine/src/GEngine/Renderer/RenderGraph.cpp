#include "GEpch.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Graphics/CommandBuffer.h"
#include "GEngine/Renderer/RenderGraph.h"
#include "GEngine/Compute/StorageBuffer.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Graphics/GraphicsResource.h"
#include "GEngine/Graphics/Texture.h"
#include <stdexcept>

namespace GEngine
{
	RenderGraph::TargetHandle RenderGraph::CreateRenderTarget(std::string name,
		const RenderPassSpecification& specification, uint32_t width, uint32_t height)
	{
		if (!width || !height || specification.RenderTargets.empty())
			throw std::invalid_argument("A graph target needs dimensions and color attachments.");
		Target target{ specification, width, height, {}, {} };
		for (size_t i = 0; i < specification.RenderTargets.size(); ++i)
		{
			const auto resource = ImportResource(name + ".Color" + std::to_string(i), ResourceState::ShaderRead);
			m_Resources[resource].IsAttachment = true;
			m_Resources[resource].AllowedStates = { ResourceState::RenderTarget, ResourceState::ShaderRead, ResourceState::CopySource };
			target.Colors.push_back(resource);
		}
		m_Targets.push_back(std::move(target));
		m_IsCompiled = false;
		return static_cast<TargetHandle>(m_Targets.size() - 1);
	}
	RenderGraph::ResourceHandle RenderGraph::GetColorAttachment(TargetHandle target, uint32_t index) const
	{
		return m_Targets.at(target).Colors.at(index);
	}
	Ref<FrameBuffer> RenderGraph::GetFrameBuffer(TargetHandle target) const
	{
		if (!m_IsCompiled) throw std::logic_error("Compile the graph before retrieving attachments.");
		return m_Targets.at(target).Object;
	}
	RenderGraph::PassBuilder RenderGraph::BuildGraphicsPass(std::string name, TargetHandle target, RecordCallback record)
	{
		if (target >= m_Targets.size() || !record) throw std::invalid_argument("Invalid graphics pass.");
		const auto pass = AddPass(std::move(name), [] {});
		m_Passes[pass].Record = std::move(record);
		m_Passes[pass].Queue = COMMAND_BUFFER_TYPE_GRAPHICS;
		m_Passes[pass].Target = target;
		for (auto color : m_Targets[target].Colors) Write(pass, color, ResourceState::RenderTarget);
		return PassBuilder(*this, pass);
	}
	RenderGraph::PassBuilder RenderGraph::BuildComputePass(std::string name, RecordCallback record)
	{
		if (!record) throw std::invalid_argument("Invalid compute pass.");
		const auto pass = AddPass(std::move(name), [] {});
		m_Passes[pass].Record = std::move(record);
		m_Passes[pass].Queue = COMMAND_BUFFER_TYPE_COMPUTE;
		return PassBuilder(*this, pass);
	}
	void RenderGraph::ExecuteGpu(const Ref<CommandBuffer>& completion)
	{
		if (!completion) throw std::invalid_argument("Graph execution requires a completion submission.");
		if (!m_IsCompiled && !Compile()) throw std::runtime_error("Render graph contains a dependency cycle.");
		std::vector<Ref<CommandBuffer>> commands(m_Passes.size());
		uint32_t graphicsCount = 0, computeCount = 0;
		for (auto handle : m_ExecutionOrder)
		{
			const auto& pass = m_Passes[handle];
			if (!pass.Record) throw std::invalid_argument("ExecuteGpu requires recording passes only.");
			if (pass.Queue == COMMAND_BUFFER_TYPE_GRAPHICS) ++graphicsCount;
			else ++computeCount;
			for (const auto& access : pass.ResourceAccesses)
				if (pass.Queue == COMMAND_BUFFER_TYPE_COMPUTE && access.State != ResourceState::ShaderWrite)
					throw std::invalid_argument("Compute graph accesses currently require storage/UAV state.");
		}
		if (graphicsCount + 1 > Graphics::GetCommandBufferCount() || computeCount > Graphics::GetCommandBufferCount())
			throw std::invalid_argument("Graph exceeds the configured per-frame command-buffer capacity.");
		for (auto handle : m_ExecutionOrder)
			commands[handle] = m_Passes[handle].Queue == COMMAND_BUFFER_TYPE_GRAPHICS
				? Graphics::GetGraphicsCommandBuffer() : Graphics::GetComputeCommandBuffer();
		// Register every edge before submitting its producer. Binary semaphore
		// backends require one independently consumable signal per edge.
		for (auto handle : m_ExecutionOrder)
		{
			auto dependencies = m_Passes[handle].Dependencies;
			dependencies.insert(dependencies.end(), m_Passes[handle].InferredDependencies.begin(), m_Passes[handle].InferredDependencies.end());
			std::sort(dependencies.begin(), dependencies.end());
			dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
			for (auto dependency : dependencies) Graphics::SetCommandsBarrier(commands[dependency], commands[handle]);
			auto finalSubmission = completion;
			Graphics::SetCommandsBarrier(commands[handle], finalSubmission);
		}
		for (auto handle : m_ExecutionOrder)
		{
			auto& pass = m_Passes[handle];
			auto& command = commands[handle];
			command->Begin();
			for (const auto& transition : pass.Transitions)
			{
				const bool attachment = pass.Target != InvalidTarget &&
					std::find(m_Targets[pass.Target].Colors.begin(), m_Targets[pass.Target].Colors.end(), transition.Resource) != m_Targets[pass.Target].Colors.end();
				if (!attachment) Graphics::TransitionResource(command, m_Resources[transition.Resource].Object, transition.Before, transition.After);
			}
			if (pass.Target != InvalidTarget) command->BeginRenderPass(m_Targets[pass.Target].Object);
			pass.Record(command);
			command->End();
			Graphics::GetRenderDevice().GetQueue(pass.Queue).Submit(command);
		}
	}
	void RenderGraph::ValidateVersion(ResourceVersion version) const
	{
		if (version.Owner != this || version.Generation != m_Generation ||
			version.Resource >= m_Resources.size() ||
			version.Version >= m_Resources[version.Resource].Versions.size())
			throw std::invalid_argument("Invalid or stale render-graph resource version.");
	}
	RenderGraph::ResourceVersion RenderGraph::GetVersion(ResourceHandle resource)
	{
		if (resource >= m_Resources.size()) throw std::invalid_argument("Invalid resource.");
		auto& entry = m_Resources[resource];
		if (!entry.UsesVersions)
		{
			for (const auto& pass : m_Passes)
				for (const auto& access : pass.ResourceAccesses)
					if (access.Resource == resource)
						throw std::invalid_argument("Cannot mix versioned and legacy resource accesses.");
			entry.UsesVersions = true;
			entry.Versions.push_back({});
		}
		return { resource, static_cast<uint32_t>(entry.Versions.size() - 1), this, m_Generation };
	}
	void RenderGraph::ReadVersion(PassHandle pass, ResourceVersion version, ResourceState state)
	{
		ValidateVersion(version);
		AddAccess(pass, version.Resource, state, false, true);
		m_Resources[version.Resource].Versions[version.Version].Readers.push_back(pass);
	}
	RenderGraph::ResourceVersion RenderGraph::WriteVersion(PassHandle pass, ResourceVersion previous, ResourceState state)
	{
		ValidateVersion(previous);
		auto& resource = m_Resources[previous.Resource];
		if (previous.Version + 1 != resource.Versions.size())
			throw std::invalid_argument("Writes must extend the latest resource version.");
		AddAccess(pass, previous.Resource, state, true, true);
		resource.Versions.push_back({ pass, {} });
		return { previous.Resource, previous.Version + 1, this, m_Generation };
	}
	void RenderGraph::BuildVersionDependencies()
	{
		auto depend = [this](PassHandle pass, PassHandle prior)
		{
			if (pass != InvalidPass && prior != InvalidPass && pass != prior)
				m_Passes[pass].InferredDependencies.push_back(prior);
		};
		for (const auto& resource : m_Resources)
		{
			if (!resource.UsesVersions) continue;
			if (resource.IsTransient && !resource.Versions.front().Readers.empty())
				throw std::invalid_argument("Cannot read uninitialized transient version: " + resource.Name);
			for (size_t index = 0; index < resource.Versions.size(); ++index)
			{
				const auto& version = resource.Versions[index];
				for (auto reader : version.Readers) depend(reader, version.Writer);
				if (index == 0) continue;
				const auto& previous = resource.Versions[index - 1];
				depend(version.Writer, previous.Writer);
				for (auto reader : previous.Readers) depend(version.Writer, reader);
			}
		}
	}
	RenderGraph::PassHandle RenderGraph::AddPass(std::string name, ExecuteCallback execute)
	{
		if (!execute) throw std::invalid_argument("Render-graph pass requires an execute callback.");
		m_IsCompiled = false;
		m_Passes.push_back({ std::move(name), std::move(execute), {}, {}, {} });
		return static_cast<PassHandle>(m_Passes.size() - 1);
	}

	RenderGraph::PassHandle RenderGraph::AddPass(std::string name, std::function<void()> execute)
	{
		if (!execute) throw std::invalid_argument("Render-graph pass requires an execute callback.");
		return AddPass(std::move(name), [execute = std::move(execute)](FrameContext&) { execute(); });
	}

	void RenderGraph::AddDependency(PassHandle pass, PassHandle dependency)
	{
		if (pass >= m_Passes.size() || dependency >= m_Passes.size() || pass == dependency)
			throw std::invalid_argument("Invalid render-graph dependency.");
		m_IsCompiled = false;
		const auto& dependencies = m_Passes[pass].Dependencies;
		if (std::find(dependencies.begin(), dependencies.end(), dependency) == dependencies.end())
			m_Passes[pass].Dependencies.push_back(dependency);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportResource(std::string name, ResourceState initialState)
	{
		GE_CORE_ASSERT(!name.empty(), "Render-graph resources require a name.");
		m_IsCompiled = false;
		m_Resources.push_back({ std::move(name), initialState, nullptr, {}, false });
		return static_cast<ResourceHandle>(m_Resources.size() - 1);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportExternalResource(std::string name, const Ref<GraphicsResource>& resource, ResourceState initialState)
	{
		GE_CORE_ASSERT(resource, "External render-graph resources require an engine resource.");
		const auto handle = ImportResource(std::move(name), initialState);
		m_Resources[handle].Object = resource;
		return handle;
	}

	RenderGraph::ResourceHandle RenderGraph::ImportTexture(std::string name, const Ref<Texture>& texture, ResourceState initialState)
	{
		GE_CORE_ASSERT(texture, "Render-graph texture imports require a texture.");
		return ImportExternalResource(std::move(name), std::static_pointer_cast<GraphicsResource>(texture), initialState);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportStorageBuffer(std::string name, const Ref<StorageBuffer>& buffer, ResourceState initialState)
	{
		GE_CORE_ASSERT(buffer, "Render-graph storage-buffer imports require a buffer.");
		return ImportExternalResource(std::move(name), std::static_pointer_cast<GraphicsResource>(buffer), initialState);
	}

	RenderGraph::ResourceHandle RenderGraph::ImportStorageImage(std::string name, const Ref<StorageImage2D>& image, ResourceState initialState)
	{
		GE_CORE_ASSERT(image, "Render-graph storage-image imports require an image.");
		return ImportExternalResource(std::move(name), std::static_pointer_cast<GraphicsResource>(image), initialState);
	}

	RenderGraph::ResourceHandle RenderGraph::CreateTransientTexture2D(std::string name, const Texture2DDesc& description, ResourceState initialState)
	{
		GE_CORE_ASSERT(description.Width > 0 && description.Height > 0, "Transient textures require a non-zero extent.");
		const auto handle = ImportResource(std::move(name), initialState);
		auto& resource = m_Resources[handle];
		resource.IsTransient = true;
		resource.AllowedStates = { ResourceState::ShaderRead, ResourceState::CopySource, ResourceState::CopyDestination };
		resource.Create = [description]()
		{
			return std::static_pointer_cast<GraphicsResource>(Texture2D::Create(description.Width, description.Height, description.Format));
		};
		return handle;
	}

	RenderGraph::ResourceHandle RenderGraph::CreateTransientStorageBuffer(std::string name, const StorageBufferDesc& description, ResourceState initialState)
	{
		GE_CORE_ASSERT(description.Size > 0, "Transient storage buffers require a non-zero size.");
		const auto handle = ImportResource(std::move(name), initialState);
		auto& resource = m_Resources[handle];
		resource.IsTransient = true;
		resource.AllowedStates = { ResourceState::ShaderRead, ResourceState::ShaderWrite, ResourceState::CopyDestination };
		resource.Create = [description]()
		{
			return std::static_pointer_cast<GraphicsResource>(StorageBuffer::Create(description.Size));
		};
		return handle;
	}

	RenderGraph::ResourceHandle RenderGraph::CreateTransientStorageImage(std::string name, const StorageImage2DDesc& description, ResourceState initialState)
	{
		GE_CORE_ASSERT(description.Width > 0 && description.Height > 0, "Transient storage images require a non-zero extent.");
		const auto handle = ImportResource(std::move(name), initialState);
		auto& resource = m_Resources[handle];
		resource.IsTransient = true;
		resource.AllowedStates = { ResourceState::ShaderRead, ResourceState::ShaderWrite, ResourceState::CopyDestination };
		resource.Create = [description]()
		{
			return std::static_pointer_cast<GraphicsResource>(StorageImage2D::Create(description.Width, description.Height, description.Format));
		};
		return handle;
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

	Ref<GraphicsResource> RenderGraph::GetResource(ResourceHandle resource) const
	{
		GE_CORE_ASSERT(resource < m_Resources.size(), "Render-graph resource is invalid.");
		return m_Resources[resource].Object;
	}

	Ref<Texture2D> RenderGraph::GetTexture2D(ResourceHandle resource) const
	{
		return std::dynamic_pointer_cast<Texture2D>(GetResource(resource));
	}

	Ref<StorageBuffer> RenderGraph::GetStorageBuffer(ResourceHandle resource) const
	{
		return std::dynamic_pointer_cast<StorageBuffer>(GetResource(resource));
	}

	Ref<StorageImage2D> RenderGraph::GetStorageImage(ResourceHandle resource) const
	{
		return std::dynamic_pointer_cast<StorageImage2D>(GetResource(resource));
	}

	const RenderGraph::ResourceLifetime& RenderGraph::GetResourceLifetime(ResourceHandle resource) const
	{
		GE_CORE_ASSERT(resource < m_Resources.size(), "Render-graph resource is invalid.");
		GE_CORE_ASSERT(m_IsCompiled, "Render-graph resource lifetime is available after compilation.");
		return m_Resources[resource].Lifetime;
	}

	bool RenderGraph::Compile()
	{
		m_IsCompiled = false;
		m_ExecutionOrder.clear();
		for (auto& pass : m_Passes)
			pass.InferredDependencies.clear();
		for (PassHandle pass = 0; pass < m_Passes.size(); ++pass)
			for (const auto& access : m_Passes[pass].ResourceAccesses)
				if (!m_Resources[access.Resource].UsesVersions)
					AddResourceDependency(pass, access.Resource, access.IsWrite);
		BuildVersionDependencies();
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

		ValidateResourceAccesses();
		CreateTransientResources();
		for (auto& target : m_Targets)
		{
			if (!target.Object)
			{
				auto& device = Graphics::GetRenderDevice();
				target.Object = device.CreateFrameBuffer(device.CreateRenderPass(target.Specification), target.Width, target.Height);
			}
			for (uint32_t i = 0; i < target.Colors.size(); ++i)
				m_Resources[target.Colors[i]].Object = target.Object->GetRenderTarget(i);
		}
		BuildResourceTransitions();
		m_IsCompiled = true;
		return true;
	}

	void RenderGraph::Execute(FrameContext& frameContext)
	{
		for (const auto& pass : m_Passes)
			if (pass.Record) throw std::invalid_argument("Recording passes require ExecuteGpu.");
		if (!m_IsCompiled && !Compile())
			throw std::runtime_error("Render graph contains a dependency cycle.");
		for (const PassHandle pass : m_ExecutionOrder)
		{
			for (const auto& transition : m_Passes[pass].Transitions)
			{
				if (m_TransitionCallback)
					m_TransitionCallback(frameContext, m_Resources[transition.Resource].Object, transition.Before, transition.After);
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
		++m_Generation;
		m_Passes.clear();
		m_Resources.clear();
		m_Targets.clear();
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
					m_Passes[pass].InferredDependencies.push_back(previous);
			}
		}
	}

	void RenderGraph::AddAccess(PassHandle pass, ResourceHandle resource, ResourceState state, bool isWrite, bool versioned)
	{
		if (pass >= m_Passes.size() || resource >= m_Resources.size() || state == ResourceState::Undefined)
			throw std::invalid_argument("Invalid render-graph resource access.");
		if (m_Resources[resource].UsesVersions != versioned)
			throw std::invalid_argument("Cannot mix versioned and legacy resource accesses.");
		if ((isWrite && (state == ResourceState::ShaderRead || state == ResourceState::CopySource)) ||
			(!isWrite && state == ResourceState::CopyDestination))
			throw std::invalid_argument("Resource state is incompatible with the declared access.");
		for (const auto& access : m_Passes[pass].ResourceAccesses)
			if (access.Resource == resource && access.State != state)
				throw std::invalid_argument("Conflicting states for one resource within a pass.");
		m_IsCompiled = false;
		m_Passes[pass].ResourceAccesses.push_back({ resource, state, isWrite });
	}

	void RenderGraph::CreateTransientResources()
	{
		for (auto& resource : m_Resources)
		{
			if (!resource.IsTransient || resource.Object != nullptr)
				continue;

			GE_CORE_ASSERT(resource.Create, "Transient render-graph resource is missing a creation callback.");
			resource.Object = resource.Create();
			GE_CORE_ASSERT(resource.Object, "Transient render-graph resource creation failed.");
		}
	}

	void RenderGraph::ValidateResourceAccesses() const
	{
		std::vector<bool> initialized(m_Resources.size(), false);
		for (ResourceHandle i = 0; i < m_Resources.size(); ++i)
		{
			const auto& resource = m_Resources[i];
			if (resource.IsTransient && resource.InitialState != ResourceState::Undefined)
				throw std::invalid_argument("Transient resource must start Undefined: " + resource.Name);
			initialized[i] = !resource.IsTransient && !resource.IsAttachment;
		}
		for (const auto pass : m_ExecutionOrder)
		{
			for (const auto& access : m_Passes[pass].ResourceAccesses)
			{
				const auto& resource = m_Resources[access.Resource];
				if (!resource.IsTransient && !resource.IsAttachment) continue;
				if (std::find(resource.AllowedStates.begin(), resource.AllowedStates.end(), access.State) == resource.AllowedStates.end())
					throw std::invalid_argument("Unsupported transient resource state: " + resource.Name);
				if (access.IsWrite && (access.State == ResourceState::ShaderRead || access.State == ResourceState::CopySource))
					throw std::invalid_argument("Write declared with a read-only state: " + resource.Name);
				if (!access.IsWrite && access.State == ResourceState::CopyDestination)
					throw std::invalid_argument("Read declared with a write-only state: " + resource.Name);
				if (!access.IsWrite && !initialized[access.Resource])
					throw std::invalid_argument("Transient resource read before first write: " + resource.Name);
				if (access.IsWrite) initialized[access.Resource] = true;
			}
		}
	}

	void RenderGraph::BuildResourceTransitions()
	{
		std::vector<ResourceState> states;
		states.reserve(m_Resources.size());
		for (auto& resource : m_Resources)
		{
			states.push_back(resource.InitialState);
			resource.Lifetime = { InvalidPass, InvalidPass, resource.InitialState };
		}

		for (const PassHandle pass : m_ExecutionOrder)
		{
			auto& transitions = m_Passes[pass].Transitions;
			transitions.clear();
			for (const auto& access : m_Passes[pass].ResourceAccesses)
			{
				auto& lifetime = m_Resources[access.Resource].Lifetime;
				if (lifetime.FirstUse == InvalidPass)
					lifetime.FirstUse = pass;
				lifetime.LastUse = pass;
				auto& currentState = states[access.Resource];
				if (currentState != access.State || currentState == ResourceState::ShaderWrite)
				{
					transitions.push_back({ access.Resource, currentState, access.State });
					currentState = access.State;
				}
				lifetime.FinalState = currentState;
			}
			if (m_Passes[pass].Target != InvalidTarget)
				for (auto color : m_Targets[m_Passes[pass].Target].Colors)
				{
					states[color] = ResourceState::ShaderRead;
					m_Resources[color].Lifetime.FinalState = ResourceState::ShaderRead;
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

		for (const PassHandle dependency : m_Passes[pass].InferredDependencies)
		{
			if (!Visit(dependency, states)) return false;
		}

		states[pass] = 2;
		m_ExecutionOrder.push_back(pass);
		return true;
	}
}

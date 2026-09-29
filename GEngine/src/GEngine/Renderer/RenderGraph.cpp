#include "GEpch.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Graphics/CommandBuffer.h"
#include "GEngine/Renderer/RenderGraph.h"
#include "GEngine/Compute/StorageBuffer.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Graphics/GraphicsResource.h"
#include "GEngine/Graphics/Texture.h"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace GEngine
{
	namespace
	{
		uint64_t RangeEnd(uint32_t base, uint32_t count)
		{
			return count == RenderGraph::SubresourceRange::All
				? std::numeric_limits<uint64_t>::max()
				: static_cast<uint64_t>(base) + count;
		}

		bool RangesOverlap(const RenderGraph::SubresourceRange& first, const RenderGraph::SubresourceRange& second)
		{
			const uint64_t firstMipEnd = RangeEnd(first.BaseMipLevel, first.MipLevelCount);
			const uint64_t secondMipEnd = RangeEnd(second.BaseMipLevel, second.MipLevelCount);
			const uint64_t firstLayerEnd = RangeEnd(first.BaseArrayLayer, first.ArrayLayerCount);
			const uint64_t secondLayerEnd = RangeEnd(second.BaseArrayLayer, second.ArrayLayerCount);
			return first.BaseMipLevel < secondMipEnd && second.BaseMipLevel < firstMipEnd &&
				first.BaseArrayLayer < secondLayerEnd && second.BaseArrayLayer < firstLayerEnd;
		}

		RenderGraph::SubresourceRange RangeIntersection(const RenderGraph::SubresourceRange& first, const RenderGraph::SubresourceRange& second)
		{
			const uint64_t mipStart = std::max(first.BaseMipLevel, second.BaseMipLevel);
			const uint64_t mipEnd = std::min(RangeEnd(first.BaseMipLevel, first.MipLevelCount),
				RangeEnd(second.BaseMipLevel, second.MipLevelCount));
			const uint64_t layerStart = std::max(first.BaseArrayLayer, second.BaseArrayLayer);
			const uint64_t layerEnd = std::min(RangeEnd(first.BaseArrayLayer, first.ArrayLayerCount),
				RangeEnd(second.BaseArrayLayer, second.ArrayLayerCount));
			return {
				static_cast<uint32_t>(mipStart),
				static_cast<uint32_t>(mipEnd - mipStart),
				static_cast<uint32_t>(layerStart),
				static_cast<uint32_t>(layerEnd - layerStart)
			};
		}

		std::vector<RenderGraph::SubresourceRange> SubtractRange(
			const RenderGraph::SubresourceRange& source, const RenderGraph::SubresourceRange& removal)
		{
			if (!RangesOverlap(source, removal))
				return { source };

			const auto intersection = RangeIntersection(source, removal);
			const uint64_t sourceMipEnd = RangeEnd(source.BaseMipLevel, source.MipLevelCount);
			const uint64_t sourceLayerEnd = RangeEnd(source.BaseArrayLayer, source.ArrayLayerCount);
			std::vector<RenderGraph::SubresourceRange> result;

			if (source.BaseMipLevel < intersection.BaseMipLevel)
				result.push_back({ source.BaseMipLevel, intersection.BaseMipLevel - source.BaseMipLevel,
					source.BaseArrayLayer, source.ArrayLayerCount });
			const uint64_t intersectionMipEnd = static_cast<uint64_t>(intersection.BaseMipLevel) + intersection.MipLevelCount;
			if (intersectionMipEnd < sourceMipEnd)
				result.push_back({ intersection.BaseMipLevel, static_cast<uint32_t>(sourceMipEnd - intersectionMipEnd),
					source.BaseArrayLayer, source.ArrayLayerCount });
			if (source.BaseArrayLayer < intersection.BaseArrayLayer)
				result.push_back({ intersection.BaseMipLevel, intersection.MipLevelCount,
					source.BaseArrayLayer, intersection.BaseArrayLayer - source.BaseArrayLayer });
			const uint64_t intersectionLayerEnd = static_cast<uint64_t>(intersection.BaseArrayLayer) + intersection.ArrayLayerCount;
			if (intersectionLayerEnd < sourceLayerEnd)
				result.push_back({ intersection.BaseMipLevel, intersection.MipLevelCount,
					intersection.BaseArrayLayer, static_cast<uint32_t>(sourceLayerEnd - intersectionLayerEnd) });
			return result;
		}
	}

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
			SetSubresourceMetadata(resource, { 1, 1 });
			target.Colors.push_back(resource);
		}
		m_Targets.push_back(std::move(target));
		m_IsCompiled = false;
		return static_cast<TargetHandle>(m_Targets.size() - 1);
	}

	RenderGraph::TargetHandle RenderGraph::CreateRenderTarget(std::string name,
		const AttachmentSpecification& specification, uint32_t width, uint32_t height)
	{
		ValidateAttachmentSpecification(specification);
		return CreateRenderTarget(std::move(name), CreateRenderPassSpecification(specification), width, height);
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
		for (auto color : m_Targets[target].Colors) Write(pass, color, ResourceState::RenderTarget, GraphicsPipelineStage::Graphics);
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
			{
				const auto& resource = m_Resources[access.Resource];
				const bool wholeRange = resource.HasSubresourceMetadata
					? access.Range.IsWholeResource(resource.SubresourceMetadata.MipLevelCount, resource.SubresourceMetadata.ArrayLayerCount)
					: access.Range.IsWholeResource();
				if (!wholeRange)
					throw std::invalid_argument("GPU subresource ranges require native range barriers on every backend.");
			}
			for (const auto& access : pass.ResourceAccesses)
				if (pass.Queue == COMMAND_BUFFER_TYPE_COMPUTE &&
					(access.Usage.State != ResourceState::ShaderWrite || access.Usage.Stage == GraphicsPipelineStage::Graphics ||
						access.Usage.Stage == GraphicsPipelineStage::Transfer))
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
				if (!attachment) Graphics::TransitionResource(command, m_Resources[transition.Resource].Object, transition.Before, transition.After, transition.Range);
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
	void RenderGraph::ReadVersion(PassHandle pass, ResourceVersion version, ResourceState state, GraphicsPipelineStage stage)
	{
		ValidateVersion(version);
		AddAccess(pass, version.Resource, state, false, stage, true);
		m_Resources[version.Resource].Versions[version.Version].Readers.push_back(pass);
	}
	RenderGraph::ResourceVersion RenderGraph::WriteVersion(PassHandle pass, ResourceVersion previous, ResourceState state, GraphicsPipelineStage stage)
	{
		ValidateVersion(previous);
		auto& resource = m_Resources[previous.Resource];
		if (previous.Version + 1 != resource.Versions.size())
			throw std::invalid_argument("Writes must extend the latest resource version.");
		AddAccess(pass, previous.Resource, state, true, stage, true);
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
		if (resource->GetResourceType() == GraphicsResourceType::Texture)
		{
			const auto metadata = resource->GetSubresourceMetadata();
			if (!metadata.MipLevelCount || !metadata.ArrayLayerCount)
				throw std::invalid_argument("Imported textures require valid subresource metadata: " + m_Resources[handle].Name);
			SetSubresourceMetadata(handle, metadata);
		}
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
		GE_CORE_ASSERT(description.MipLevelCount > 0, "Transient textures require a non-zero mip count.");
		GE_CORE_ASSERT(description.ArrayLayerCount == 1, "A transient 2D texture supports one array layer.");
		const auto handle = ImportResource(std::move(name), initialState);
		auto& resource = m_Resources[handle];
		resource.IsTransient = true;
		resource.AllowedStates = { ResourceState::ShaderRead, ResourceState::CopySource, ResourceState::CopyDestination };
		SetSubresourceMetadata(handle, { description.MipLevelCount, description.ArrayLayerCount });
		resource.Create = [description]()
		{
			return std::static_pointer_cast<GraphicsResource>(Texture2D::Create(
				description.Width, description.Height, description.Format, description.MipLevelCount));
		};
		return handle;
	}

	RenderGraph::ResourceHandle RenderGraph::CreateTransientTexture2DArray(std::string name, const Texture2DArrayDesc& description, ResourceState initialState)
	{
		GE_CORE_ASSERT(description.Width > 0 && description.Height > 0, "Transient texture arrays require a non-zero extent.");
		GE_CORE_ASSERT(description.ArrayLayerCount > 0, "Transient texture arrays require a non-zero layer count.");
		GE_CORE_ASSERT(description.MipLevelCount == 1, "Transient texture arrays currently support one mip level.");
		const auto handle = ImportResource(std::move(name), initialState);
		auto& resource = m_Resources[handle];
		resource.IsTransient = true;
		resource.AllowedStates = { ResourceState::ShaderRead, ResourceState::CopySource, ResourceState::CopyDestination };
		SetSubresourceMetadata(handle, { description.MipLevelCount, description.ArrayLayerCount });
		resource.Create = [description]()
		{
			return std::static_pointer_cast<GraphicsResource>(Texture2DArray::Create(
				description.Width, description.Height, description.ArrayLayerCount, description.Format));
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
		GE_CORE_ASSERT(description.MipLevelCount == 1, "Transient storage images currently support one mip level.");
		GE_CORE_ASSERT(description.ArrayLayerCount == 1, "Transient storage images currently support one array layer.");
		const auto handle = ImportResource(std::move(name), initialState);
		auto& resource = m_Resources[handle];
		resource.IsTransient = true;
		resource.AllowedStates = { ResourceState::ShaderRead, ResourceState::ShaderWrite, ResourceState::CopyDestination };
		SetSubresourceMetadata(handle, { description.MipLevelCount, description.ArrayLayerCount });
		resource.Create = [description]()
		{
			return std::static_pointer_cast<GraphicsResource>(StorageImage2D::Create(description.Width, description.Height, description.Format));
		};
		return handle;
	}

	void RenderGraph::Read(PassHandle pass, ResourceHandle resource, ResourceState state, GraphicsPipelineStage stage, SubresourceRange range)
	{
		AddAccess(pass, resource, state, false, stage, false, range);
	}

	void RenderGraph::Write(PassHandle pass, ResourceHandle resource, ResourceState state, GraphicsPipelineStage stage, SubresourceRange range)
	{
		AddAccess(pass, resource, state, true, stage, false, range);
	}

	void RenderGraph::SetTransitionCallback(TransitionCallback callback)
	{
		m_TransitionCallback = std::move(callback);
	}

	void RenderGraph::SetTransitionUsageCallback(UsageTransitionCallback callback)
	{
		m_UsageTransitionCallback = std::move(callback);
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
		ValidateSubresourceRanges();
		for (PassHandle pass = 0; pass < m_Passes.size(); ++pass)
			for (const auto& access : m_Passes[pass].ResourceAccesses)
				if (!m_Resources[access.Resource].UsesVersions)
					AddResourceDependency(pass, access.Resource, access.IsWrite, access.Range);
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
			if (!target.Specification.Subpasses.empty() && !Graphics::GetCapabilities().Subpasses)
				throw std::invalid_argument("Render-graph subpasses are not supported by the active backend.");
			if (!target.Object)
			{
				auto& device = Graphics::GetRenderDevice();
				const auto renderPass = device.CreateRenderPass(target.Specification);
				target.Object = device.CreateFrameBuffer(renderPass, target.Width, target.Height);
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
				if (m_UsageTransitionCallback)
					m_UsageTransitionCallback(frameContext, m_Resources[transition.Resource].Object,
						transition.Before, transition.After, transition.Range);
				if (m_TransitionCallback)
					m_TransitionCallback(frameContext, m_Resources[transition.Resource].Object,
						transition.Before.State, transition.After.State);
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

	void RenderGraph::AddResourceDependency(PassHandle pass, ResourceHandle resource, bool isWrite, const SubresourceRange& range)
	{
		const bool supportsIndependentSubresourceScheduling = false;
		for (PassHandle previous = 0; previous < pass; ++previous)
		{
			for (const auto& access : m_Passes[previous].ResourceAccesses)
			{
				if (access.Resource == resource && (isWrite || access.IsWrite) &&
					(RangesOverlap(access.Range, range) || !supportsIndependentSubresourceScheduling))
					m_Passes[pass].InferredDependencies.push_back(previous);
			}
		}
	}

	void RenderGraph::AddAccess(PassHandle pass, ResourceHandle resource, ResourceState state, bool isWrite,
		GraphicsPipelineStage stage, bool versioned, SubresourceRange range)
	{
		if (pass >= m_Passes.size() || resource >= m_Resources.size() || state == ResourceState::Undefined)
			throw std::invalid_argument("Invalid render-graph resource access.");
		if (m_Resources[resource].UsesVersions != versioned)
			throw std::invalid_argument("Cannot mix versioned and legacy resource accesses.");
		if ((isWrite && (state == ResourceState::ShaderRead || state == ResourceState::CopySource)) ||
			(!isWrite && state == ResourceState::CopyDestination))
			throw std::invalid_argument("Resource state is incompatible with the declared access.");
		if (!range.MipLevelCount || !range.ArrayLayerCount)
			throw std::invalid_argument("Render-graph subresource ranges must not be empty.");
		if ((range.MipLevelCount != SubresourceRange::All && range.BaseMipLevel > std::numeric_limits<uint32_t>::max() - range.MipLevelCount) ||
			(range.ArrayLayerCount != SubresourceRange::All && range.BaseArrayLayer > std::numeric_limits<uint32_t>::max() - range.ArrayLayerCount))
			throw std::invalid_argument("Render-graph subresource range overflows.");
		for (const auto& access : m_Passes[pass].ResourceAccesses)
			if (access.Resource == resource && (access.Usage.State != state || access.Usage.Stage != stage))
				throw std::invalid_argument("Conflicting states for one resource within a pass.");
		m_IsCompiled = false;
		m_Passes[pass].ResourceAccesses.push_back({ resource, { state, stage,
			isWrite ? GraphicsResourceAccess::Write : GraphicsResourceAccess::Read }, range, isWrite });
	}

	void RenderGraph::SetSubresourceMetadata(ResourceHandle resource, GraphicsSubresourceMetadata metadata)
	{
		if (resource >= m_Resources.size())
			throw std::invalid_argument("Invalid render-graph resource.");
		if (!metadata.MipLevelCount || !metadata.ArrayLayerCount)
			throw std::invalid_argument("Render-graph subresource metadata must not be empty: " + m_Resources[resource].Name);
		m_Resources[resource].HasSubresourceMetadata = true;
		m_Resources[resource].SubresourceMetadata = metadata;
	}

	void RenderGraph::ValidateSubresourceRanges()
	{
		for (const auto& resource : m_Resources)
			if (resource.HasSubresourceMetadata &&
				(!resource.SubresourceMetadata.MipLevelCount || !resource.SubresourceMetadata.ArrayLayerCount))
				throw std::invalid_argument("Render-graph subresource metadata must not be empty: " + resource.Name);

		for (auto& pass : m_Passes)
		{
			for (auto& access : pass.ResourceAccesses)
			{
				const auto& resource = m_Resources[access.Resource];
				if (!resource.HasSubresourceMetadata)
				{
					if (!access.Range.IsWholeResource())
						throw std::invalid_argument("Render-graph resource lacks subresource metadata: " + resource.Name);
					continue;
				}

				const auto metadata = resource.SubresourceMetadata;
				if (access.Range.MipLevelCount == SubresourceRange::All)
					access.Range.MipLevelCount = metadata.MipLevelCount;
				if (access.Range.ArrayLayerCount == SubresourceRange::All)
					access.Range.ArrayLayerCount = metadata.ArrayLayerCount;
				if (access.Range.BaseMipLevel >= metadata.MipLevelCount ||
					access.Range.BaseArrayLayer >= metadata.ArrayLayerCount)
					throw std::invalid_argument("Render-graph subresource range is out of bounds: " + resource.Name);
				if (!access.Range.MipLevelCount || !access.Range.ArrayLayerCount)
					throw std::invalid_argument("Render-graph subresource range is empty: " + resource.Name);
				if (access.Range.BaseMipLevel > std::numeric_limits<uint32_t>::max() - access.Range.MipLevelCount ||
					access.Range.BaseArrayLayer > std::numeric_limits<uint32_t>::max() - access.Range.ArrayLayerCount)
					throw std::invalid_argument("Render-graph subresource range overflows: " + resource.Name);
			}
		}
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
				if (std::find(resource.AllowedStates.begin(), resource.AllowedStates.end(), access.Usage.State) == resource.AllowedStates.end())
					throw std::invalid_argument("Unsupported transient resource state: " + resource.Name);
				if (access.IsWrite && (access.Usage.State == ResourceState::ShaderRead || access.Usage.State == ResourceState::CopySource))
					throw std::invalid_argument("Write declared with a read-only state: " + resource.Name);
				if (!access.IsWrite && access.Usage.State == ResourceState::CopyDestination)
					throw std::invalid_argument("Read declared with a write-only state: " + resource.Name);
				if (!access.IsWrite && !initialized[access.Resource])
					throw std::invalid_argument("Transient resource read before first write: " + resource.Name);
				if (access.IsWrite) initialized[access.Resource] = true;
			}
		}
	}

	void RenderGraph::BuildResourceTransitions()
	{
		struct SubresourceUsage
		{
			GraphicsResourceUsage Usage;
			SubresourceRange Range;
		};
		auto fullRange = [](const Resource& resource)
		{
			return resource.HasSubresourceMetadata
				? SubresourceRange{ 0, resource.SubresourceMetadata.MipLevelCount, 0, resource.SubresourceMetadata.ArrayLayerCount }
				: SubresourceRange{};
		};
		auto replaceRangeState = [](std::vector<SubresourceUsage>& states, const SubresourceRange& range, GraphicsResourceUsage usage)
		{
			std::vector<SubresourceUsage> next;
			next.reserve(states.size() + 1);
			for (const auto& state : states)
			{
				for (const auto& remainder : SubtractRange(state.Range, range))
					next.push_back({ state.Usage, remainder });
			}
			next.push_back({ usage, range });
			states = std::move(next);
		};

		std::vector<std::vector<SubresourceUsage>> states(m_Resources.size());
		for (size_t index = 0; index < m_Resources.size(); ++index)
		{
			auto& resource = m_Resources[index];
			states[index].push_back({ { resource.InitialState, GraphicsPipelineStage::All,
				GraphicsResourceAccess::ReadWrite }, fullRange(resource) });
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

				std::vector<ResourceTransition> accessTransitions;
				bool stateChanged = false;
				for (const auto& state : states[access.Resource])
				{
					if (!RangesOverlap(state.Range, access.Range))
						continue;
					if (state.Usage.State != access.Usage.State || access.Usage.State == ResourceState::ShaderWrite)
					{
						accessTransitions.push_back({ access.Resource, state.Usage, access.Usage,
							RangeIntersection(state.Range, access.Range) });
						stateChanged = true;
					}
				}
				transitions.insert(transitions.end(), accessTransitions.begin(), accessTransitions.end());
				if (stateChanged)
					replaceRangeState(states[access.Resource], access.Range, access.Usage);
				lifetime.FinalState = access.Usage.State;
			}
			if (m_Passes[pass].Target != InvalidTarget)
				for (auto color : m_Targets[m_Passes[pass].Target].Colors)
				{
					replaceRangeState(states[color], fullRange(m_Resources[color]),
						{ ResourceState::ShaderRead, GraphicsPipelineStage::Graphics, GraphicsResourceAccess::Read });
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

	RenderPassSpecification RenderGraph::CreateRenderPassSpecification(const AttachmentSpecification& specification)
	{
		RenderPassSpecification result{};
		result.RenderTargets = specification.ColorFormats;
		result.DepthStencil = specification.DepthStencilFormat;
		result.Samples = specification.Samples;
		result.Operation = specification.Operation;
		result.Subpasses.reserve(specification.Subpasses.size());
		for (const auto& subpass : specification.Subpasses)
		{
			result.Subpasses.push_back({ subpass.ColorAttachmentIndices,
				subpass.InputAttachmentIndices, subpass.EnableDepthStencil });
		}
		return result;
	}

	void RenderGraph::ValidateAttachmentSpecification(const AttachmentSpecification& specification)
	{
		if (specification.ColorFormats.empty())
			throw std::invalid_argument("A portable graph attachment needs at least one color format.");
		if (std::find(specification.ColorFormats.begin(), specification.ColorFormats.end(),
			FRAME_BUFFER_TEXTURE_FORMAT_NONE) != specification.ColorFormats.end())
			throw std::invalid_argument("Portable graph color formats cannot be none.");
		if (specification.Samples == 0)
			throw std::invalid_argument("Portable graph attachments require a non-zero sample count.");
		if (specification.DepthStencilFormat != FRAME_BUFFER_TEXTURE_FORMAT_NONE &&
			!Utils::isDepthFormat(specification.DepthStencilFormat))
			throw std::invalid_argument("Portable graph depth format must be a depth-stencil format.");

		const uint32_t depthAttachmentIndex = static_cast<uint32_t>(specification.ColorFormats.size());
		for (const auto& subpass : specification.Subpasses)
		{
			for (uint32_t index : subpass.ColorAttachmentIndices)
				if (index >= specification.ColorFormats.size())
					throw std::invalid_argument("Portable graph subpass color attachment index is out of range.");
			for (uint32_t index : subpass.InputAttachmentIndices)
				if (index >= depthAttachmentIndex ||
					(index == depthAttachmentIndex && specification.DepthStencilFormat == FRAME_BUFFER_TEXTURE_FORMAT_NONE))
					throw std::invalid_argument("Portable graph subpass input attachment index is out of range.");
			if (subpass.EnableDepthStencil && specification.DepthStencilFormat == FRAME_BUFFER_TEXTURE_FORMAT_NONE)
				throw std::invalid_argument("Portable graph subpass cannot enable a missing depth attachment.");
			if (subpass.ColorAttachmentIndices.empty() && subpass.InputAttachmentIndices.empty())
				throw std::invalid_argument("Portable graph subpass needs an attachment.");
		}
	}
}

#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include "GEngine/Graphics/ImageFormat.h"
#include "GEngine/Renderer/FrameContext.h"

#include <functional>
#include <string>
#include <vector>
#include <stdexcept>

namespace GEngine
{
	class Texture;
	class Texture2D;
	class StorageBuffer;
	class StorageImage2D;
	class GraphicsResource;
	class CommandBuffer;
	class FrameBuffer;

	class GENGINE_API RenderGraph
	{
	public:
		using PassHandle = uint32_t;
		using ResourceHandle = uint32_t;
		using TargetHandle = uint32_t;
		using RecordCallback = std::function<void(const Ref<CommandBuffer>&)>;

		static constexpr PassHandle InvalidPass = UINT32_MAX;
		static constexpr ResourceHandle InvalidResource = UINT32_MAX;
		static constexpr TargetHandle InvalidTarget = UINT32_MAX;

		struct SubpassSpecification
		{
			std::vector<uint32_t> ColorAttachmentIndices;
			std::vector<uint32_t> InputAttachmentIndices;
			bool EnableDepthStencil = true;
		};

		struct AttachmentSpecification
		{
			std::vector<FrameBufferTextureFormat> ColorFormats;
			FrameBufferTextureFormat DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_NONE;
			uint32_t Samples = 1;
			RenderPassOperation Operation{};
			std::vector<SubpassSpecification> Subpasses;
		};

		TargetHandle CreateRenderTarget(std::string name, const RenderPassSpecification& specification, uint32_t width, uint32_t height);
		TargetHandle CreateRenderTarget(std::string name, const AttachmentSpecification& specification, uint32_t width, uint32_t height);
		ResourceHandle GetColorAttachment(TargetHandle target, uint32_t index = 0) const;
		Ref<FrameBuffer> GetFrameBuffer(TargetHandle target) const;

		using ResourceState = GraphicsResourceState;
		struct ResourceVersion
		{
			ResourceHandle Resource = InvalidResource;
			uint32_t Version = 0;
			const RenderGraph* Owner = nullptr;
			uint64_t Generation = 0;
		};
		ResourceVersion GetVersion(ResourceHandle resource);
		void ReadVersion(PassHandle pass, ResourceVersion resource, ResourceState state = ResourceState::ShaderRead);
		ResourceVersion WriteVersion(PassHandle pass, ResourceVersion previous, ResourceState state);

		class PassBuilder
		{
		public:
			PassBuilder(RenderGraph& graph, PassHandle pass) : m_Graph(graph), m_Pass(pass), m_Generation(graph.m_Generation) {}
			PassBuilder& DependsOn(PassHandle pass) { m_Graph.AddDependency(GetHandle(), pass); return *this; }
			PassBuilder& Read(ResourceVersion version, ResourceState state = ResourceState::ShaderRead)
			{ m_Graph.ReadVersion(GetHandle(), version, state); return *this; }
			ResourceVersion Write(ResourceVersion version, ResourceState state)
			{ return m_Graph.WriteVersion(GetHandle(), version, state); }
			PassHandle GetHandle() const
			{
				if (m_Generation != m_Graph.m_Generation || m_Pass >= m_Graph.m_Passes.size())
					throw std::invalid_argument("Stale graph pass builder.");
				return m_Pass;
			}
		private:
			RenderGraph& m_Graph;
			PassHandle m_Pass;
			uint64_t m_Generation;
		};

		using ExecuteCallback = std::function<void(FrameContext&)>;
		PassBuilder BuildGraphicsPass(std::string name, TargetHandle target, RecordCallback record);
		PassBuilder BuildComputePass(std::string name, RecordCallback record);
		void ExecuteGpu(const Ref<CommandBuffer>& completion);
		using TransitionCallback = std::function<void(const FrameContext&, const Ref<GraphicsResource>&, ResourceState, ResourceState)>;

		struct Texture2DDesc
		{
			uint32_t Width = 0;
			uint32_t Height = 0;
			RenderImage2DFormat Format = RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM;
		};

		struct StorageBufferDesc
		{
			uint32_t Size = 0;
		};

		struct StorageImage2DDesc
		{
			uint32_t Width = 0;
			uint32_t Height = 0;
			ComputeImage2DFormat Format = COMPUTE_IMAGE_2D_FORMAT_RGBA32_SFLOAT;
		};

		struct ResourceLifetime
		{
			PassHandle FirstUse = InvalidPass;
			PassHandle LastUse = InvalidPass;
			ResourceState FinalState = ResourceState::Undefined;
		};

		PassHandle AddPass(std::string name, ExecuteCallback execute);
		PassBuilder BuildPass(std::string name, ExecuteCallback execute)
		{ return PassBuilder(*this, AddPass(std::move(name), std::move(execute))); }
		PassHandle AddPass(std::string name, std::function<void()> execute);
		void AddDependency(PassHandle pass, PassHandle dependency);
		ResourceHandle ImportResource(std::string name, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle ImportExternalResource(std::string name, const Ref<GraphicsResource>& resource, ResourceState initialState);
		ResourceHandle ImportTexture(std::string name, const Ref<Texture>& texture, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle ImportStorageBuffer(std::string name, const Ref<StorageBuffer>& buffer, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle ImportStorageImage(std::string name, const Ref<StorageImage2D>& image, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle CreateTransientTexture2D(std::string name, const Texture2DDesc& description, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle CreateTransientStorageBuffer(std::string name, const StorageBufferDesc& description, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle CreateTransientStorageImage(std::string name, const StorageImage2DDesc& description, ResourceState initialState = ResourceState::Undefined);
		void Read(PassHandle pass, ResourceHandle resource, ResourceState state = ResourceState::ShaderRead);
		void Write(PassHandle pass, ResourceHandle resource, ResourceState state);
		void SetTransitionCallback(TransitionCallback callback);
		Ref<GraphicsResource> GetResource(ResourceHandle resource) const;
		Ref<Texture2D> GetTexture2D(ResourceHandle resource) const;
		Ref<StorageBuffer> GetStorageBuffer(ResourceHandle resource) const;
		Ref<StorageImage2D> GetStorageImage(ResourceHandle resource) const;
		const ResourceLifetime& GetResourceLifetime(ResourceHandle resource) const;
		bool Compile();
		void Execute(FrameContext& frameContext);
		void Execute();
		void Reset();

	private:
		struct ResourceAccess
		{
			ResourceHandle Resource;
			ResourceState State;
			bool IsWrite;
		};

		struct VersionInfo
		{
			PassHandle Writer = InvalidPass;
			std::vector<PassHandle> Readers;
		};
		struct Resource
		{
			std::string Name;
			ResourceState InitialState;
			Ref<GraphicsResource> Object;
			std::function<Ref<GraphicsResource>()> Create;
			bool IsTransient = false;
			bool IsAttachment = false;
			ResourceLifetime Lifetime;
			std::vector<ResourceState> AllowedStates;
			bool UsesVersions = false;
			std::vector<VersionInfo> Versions;
		};

		struct ResourceTransition
		{
			ResourceHandle Resource;
			ResourceState Before;
			ResourceState After;
		};

		struct Pass
		{
			std::string Name;
			ExecuteCallback Execute;
			std::vector<PassHandle> Dependencies;
			std::vector<ResourceAccess> ResourceAccesses;
			std::vector<ResourceTransition> Transitions;
			std::vector<PassHandle> InferredDependencies;
			RecordCallback Record;
			CommandBufferType Queue = COMMAND_BUFFER_TYPE_NONE;
			TargetHandle Target = InvalidTarget;
		};
		struct Target
		{
			RenderPassSpecification Specification;
			uint32_t Width, Height;
			std::vector<ResourceHandle> Colors;
			Ref<FrameBuffer> Object;
		};

		void AddResourceDependency(PassHandle pass, ResourceHandle resource, bool isWrite);
		void AddAccess(PassHandle pass, ResourceHandle resource, ResourceState state, bool isWrite, bool versioned = false);
		void ValidateVersion(ResourceVersion version) const;
		void BuildVersionDependencies();
		void CreateTransientResources();
		void ValidateResourceAccesses() const;
		void BuildResourceTransitions();
		bool Visit(PassHandle pass, std::vector<uint8_t>& states);
		static RenderPassSpecification CreateRenderPassSpecification(const AttachmentSpecification& specification);
		static void ValidateAttachmentSpecification(const AttachmentSpecification& specification);

	private:
		std::vector<Pass> m_Passes;
		std::vector<Resource> m_Resources;
		std::vector<Target> m_Targets;
		std::vector<PassHandle> m_ExecutionOrder;
		TransitionCallback m_TransitionCallback;
		bool m_IsCompiled = false;
		uint64_t m_Generation = 1;
	};
}

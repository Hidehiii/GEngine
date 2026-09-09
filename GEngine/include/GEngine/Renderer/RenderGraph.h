#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"
#include "GEngine/Graphics/ImageFormat.h"
#include "GEngine/Renderer/FrameContext.h"

#include <functional>
#include <string>
#include <vector>

namespace GEngine
{
	class Texture;
	class Texture2D;
	class StorageBuffer;
	class StorageImage2D;
	class GraphicsResource;

	class GENGINE_API RenderGraph
	{
	public:
		using PassHandle = uint32_t;
		using ResourceHandle = uint32_t;

		static constexpr PassHandle InvalidPass = UINT32_MAX;
		static constexpr ResourceHandle InvalidResource = UINT32_MAX;

		using ResourceState = GraphicsResourceState;

		using ExecuteCallback = std::function<void(FrameContext&)>;
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

		struct Resource
		{
			std::string Name;
			ResourceState InitialState;
			Ref<GraphicsResource> Object;
			std::function<Ref<GraphicsResource>()> Create;
			bool IsTransient = false;
			ResourceLifetime Lifetime;
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
		};

		void AddResourceDependency(PassHandle pass, ResourceHandle resource, bool isWrite);
		void AddAccess(PassHandle pass, ResourceHandle resource, ResourceState state, bool isWrite);
		void CreateTransientResources();
		void BuildResourceTransitions();
		bool Visit(PassHandle pass, std::vector<uint8_t>& states);

	private:
		std::vector<Pass> m_Passes;
		std::vector<Resource> m_Resources;
		std::vector<PassHandle> m_ExecutionOrder;
		TransitionCallback m_TransitionCallback;
		bool m_IsCompiled = false;
	};
}

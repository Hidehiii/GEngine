#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/FrameContext.h"

#include <functional>
#include <string>
#include <vector>

namespace GEngine
{
	class Texture;
	class StorageBuffer;
	class StorageImage2D;

	class GENGINE_API RenderGraph
	{
	public:
		using PassHandle = uint32_t;
		using ResourceHandle = uint32_t;

		static constexpr PassHandle InvalidPass = UINT32_MAX;
		static constexpr ResourceHandle InvalidResource = UINT32_MAX;

		enum class ResourceState : uint8_t
		{
			Undefined,
			RenderTarget,
			DepthWrite,
			ShaderRead,
			ShaderWrite,
			CopySource,
			CopyDestination,
			Present
		};

		using ExecuteCallback = std::function<void(FrameContext&)>;
		using TransitionCallback = std::function<void(const FrameContext&, ResourceHandle, ResourceState, ResourceState)>;

		PassHandle AddPass(std::string name, ExecuteCallback execute);
		PassHandle AddPass(std::string name, std::function<void()> execute);
		void AddDependency(PassHandle pass, PassHandle dependency);
		ResourceHandle ImportResource(std::string name, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle ImportExternalResource(std::string name, void* nativeResource, ResourceState initialState);
		ResourceHandle ImportTexture(std::string name, const Ref<Texture>& texture, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle ImportStorageBuffer(std::string name, const Ref<StorageBuffer>& buffer, ResourceState initialState = ResourceState::Undefined);
		ResourceHandle ImportStorageImage(std::string name, const Ref<StorageImage2D>& image, ResourceState initialState = ResourceState::Undefined);
		void Read(PassHandle pass, ResourceHandle resource, ResourceState state = ResourceState::ShaderRead);
		void Write(PassHandle pass, ResourceHandle resource, ResourceState state);
		void SetTransitionCallback(TransitionCallback callback);
		void* GetNativeResource(ResourceHandle resource) const;
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
			void* NativeResource = nullptr;
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

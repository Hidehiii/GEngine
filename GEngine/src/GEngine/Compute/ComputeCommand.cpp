#include "GEpch.h"
#include "ComputeCommand.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
namespace GEngine
{
	inline Vector3 ComputeCommand::GetMaxComputeWorkGroupCount()
	{
		return RenderCommand::GetMaxComputeWorkGroupCount();
	}
	inline Vector3 ComputeCommand::GetMaxComputeWorkGroupSize()
	{
		return RenderCommand::GetMaxComputeWorkGroupSize();
	}
	inline uint32_t ComputeCommand::GetMaxComputeWorkGroupInvocations()
	{
		return RenderCommand::GetMaxComputeWorkGroupInvocations();
	}
	inline void ComputeCommand::BeginComputeCommand()
	{
		RenderCommand::BeginComputeCommand();
	}
	inline void ComputeCommand::EndComputeCommand()
	{
		RenderCommand::EndComputeCommand();
	}
	inline void ComputeCommand::BeginSecondaryCommand()
	{
		RenderCommand::BeginSecondaryCommand();
	}
	inline void ComputeCommand::EndSecondaryCommand()
	{
		RenderCommand::EndSecondaryCommand();
	}
	inline void ComputeCommand::Compute(const uint32_t x, const uint32_t y, const uint32_t z)
	{
		RenderCommand::Compute(x, y, z);
	}
	inline Ref<CommandBuffer> ComputeCommand::BeginComputeCommand(Ref<FrameBuffer>& buffer, const Camera& camera)
	{
		return RenderCommand::BeginComputeCommand(buffer, camera);
	}
	inline Ref<CommandBuffer> ComputeCommand::BeginComputeCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera)
	{
		return RenderCommand::BeginComputeCommand(buffer, camera);
	}
	inline void ComputeCommand::EndComputeCommand(Ref<CommandBuffer> buffer)
	{
		RenderCommand::EndComputeCommand(buffer);
	}
	inline void ComputeCommand::Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z)
	{
		RenderCommand::Compute(buffer, x, y, z);
	}
}


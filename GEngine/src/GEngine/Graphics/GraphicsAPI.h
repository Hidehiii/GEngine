#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/GraphicsBuffer.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include "Material.h"

namespace GEngine
{
	class CommandBuffer;
	class FrameBuffer;

	class GENGINE_API GraphicsAPI
	{
	public:
		enum class API
		{
			None = 0, 
			OpenGL = 1,
			Vulkan = 2,
			Direct3DX12 = 3
		};

	public:

		virtual std::vector<std::string> GetExtensions() = 0;

		virtual uint32_t GetMaxTextureSize() = 0;
		virtual uint32_t GetMaxCombinedTextureCount() = 0;
		virtual uint32_t GetMaxPerStageTextureCount() = 0;
		virtual uint32_t GetMaxTextureArrayLayers() = 0;

		virtual uint32_t GetMinUniformBufferOffsetAlignment() = 0;
		virtual uint32_t GetMaxUniformBufferSize() = 0;

		virtual Ref<CommandBuffer> GetGraphicsCommandBuffer() = 0;
		virtual Ref<CommandBuffer> GetComputeCommandBuffer() = 0;

		// compute
		virtual Vector3 GetMaxComputeWorkGroupCount() = 0;
		virtual Vector3 GetMaxComputeWorkGroupSize() = 0;
		virtual uint32_t GetMaxComputeWorkGroupInvocations() = 0;

		virtual void SetCommandsBarrier(Ref<CommandBuffer>& first, Ref<CommandBuffer>& second) = 0;


		inline static API GetAPI() { return s_API; }
	protected:
		inline static API s_API		= API::None;
	};

}




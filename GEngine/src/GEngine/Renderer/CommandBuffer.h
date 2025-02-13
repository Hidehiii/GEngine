#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Renderer/GraphicsPipeline.h"
#include "GEngine/Compute/ComputePipeline.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "GEngine/Components/Camera/Camera.h"
#include "GEngine/Components/Camera/EditorCamera.h"

namespace GEngine
{
	class GENGINE_API CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera) = 0;
		virtual void Begin(Ref<FrameBuffer>& buffer, const Camera& camera) = 0;

		virtual void End() = 0;

		virtual void Render(Ref<Scene>& scene) = 0;
		virtual void Render(Ref<GraphicsPipeline>& pipeline) = 0;

		virtual void Compute(Ref<ComputePipeline>& pipeline) = 0;
	};
}



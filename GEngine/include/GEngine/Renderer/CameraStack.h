#pragma once
#include "GEngine/Renderer/RenderPipeline.h"
#include "GEngine/Graphics/CommandBuffer.h"
#include <functional>

namespace GEngine
{
	class RenderPipelineAssetContext;
	enum class CameraStackRole { Base, Overlay };
	struct CameraStackViewport { float X = 0, Y = 0, Width = 1, Height = 1; };
	struct CameraStackNode
	{
		std::string Camera, Target;
		CameraStackRole Role = CameraStackRole::Overlay;
		bool Enabled = true;
		uint32_t Samples = 0;
		CameraStackViewport Viewport;
		RenderPassBeginOperation Color = RENDER_PASS_BEGINE_OP_LOAD;
		RenderPassBeginOperation Depth = RENDER_PASS_BEGINE_OP_LOAD;
		std::vector<std::string> Overlays;
	};
	struct GENGINE_API CameraStackSpecification
	{
		std::string Name, Target, BaseCamera;
		std::vector<CameraStackNode> Nodes;
		std::vector<size_t> Validate(uint32_t width, uint32_t height, uint32_t samples, bool hasDepth) const;
		GraphicsViewport ResolveViewport(const CameraStackViewport& viewport, uint32_t width, uint32_t height) const;
	};
	struct CameraStackBinding
	{
		std::string Camera;
		CameraComponent* Component = nullptr;
		bool Active = true;
	};
	using CameraStackBindings = std::function<std::vector<CameraStackBinding>(RenderPipelineContext&)>;
	using CameraStackRecorder = std::function<void(const Ref<CommandBuffer>&, CameraComponent&, const CameraStackNode&)>;
	class GENGINE_API CameraStackPass final : public RenderPipelinePass
	{
	public:
		CameraStackPass(std::string name, CameraStackSpecification stack, Ref<const RenderPipelineAssetContext> context,
			CameraStackBindings bindings, CameraStackRecorder record);
		const std::string& GetName() const override { return m_Name; }
		void Configure(RenderPipelineBuilder& builder) override;
		void Prepare(const RenderGraph& graph) override;
		void Execute(RenderPipelineContext& context) override;
		void Present(RenderPipelineContext&) override { m_Views.clear(); }
	private:
		struct View { CameraComponent* Camera = nullptr; bool Active = false; GraphicsViewport Viewport; };
		void Record(size_t index, const Ref<CommandBuffer>& command);
		std::string m_Name;
		CameraStackSpecification m_Stack;
		Ref<const RenderPipelineAssetContext> m_Context;
		CameraStackBindings m_Bindings;
		CameraStackRecorder m_Record;
		std::vector<size_t> m_Order;
		std::vector<View> m_Views;
		std::vector<Ref<RenderPass>> m_RenderPasses;
		Ref<FrameBuffer> m_FrameBuffer;
	};
}

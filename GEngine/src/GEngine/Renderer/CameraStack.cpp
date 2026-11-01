#include "GEpch.h"
#include "GEngine/Renderer/CameraStack.h"
#include "GEngine/Renderer/RenderPipelineAsset.h"
#include "GEngine/Graphics/Graphics.h"
#include <cmath>
#include <map>

namespace GEngine
{
	namespace
	{
		void RequireStack(bool valid, const std::string& message)
		{
			if (!valid) throw std::invalid_argument("Camera stack: " + message);
		}
	}
	GraphicsViewport CameraStackSpecification::ResolveViewport(const CameraStackViewport& v, uint32_t width, uint32_t height) const
	{
		RequireStack(width && height && std::isfinite(v.X) && std::isfinite(v.Y) && std::isfinite(v.Width) && std::isfinite(v.Height) &&
			v.X >= 0 && v.Y >= 0 && v.Width > 0 && v.Height > 0 && double(v.X) + v.Width <= 1 && double(v.Y) + v.Height <= 1, "invalid normalized viewport.");
		const auto x = uint32_t(std::llround(double(v.X) * width)), y = uint32_t(std::llround(double(v.Y) * height));
		const auto right = uint32_t(std::llround((double(v.X) + v.Width) * width)), bottom = uint32_t(std::llround((double(v.Y) + v.Height) * height));
		RequireStack(right > x && bottom > y, "viewport rounds to an empty pixel rectangle.");
		return { x, y, right - x, bottom - y };
	}
	std::vector<size_t> CameraStackSpecification::Validate(uint32_t width, uint32_t height, uint32_t samples, bool hasDepth) const
	{
		RequireStack(!Name.empty() && !Target.empty() && !Nodes.empty() && Nodes.size() <= 64, "name, target and 1..64 nodes are required.");
		std::map<std::string,size_t> nodes;
		for (size_t i = 0; i < Nodes.size(); ++i)
		{
			const auto& node = Nodes[i];
			RequireStack(!node.Camera.empty() && nodes.emplace(node.Camera,i).second, "duplicate or empty camera ID.");
			RequireStack(node.Role == CameraStackRole::Base || node.Role == CameraStackRole::Overlay, "unknown camera role.");
			RequireStack(node.Target.empty() || node.Target == Target, "overlay target differs from the stack target.");
			RequireStack(!node.Samples || node.Samples == samples, "incompatible camera sample count.");
			RequireStack(node.Color == RENDER_PASS_BEGINE_OP_CLEAR || node.Color == RENDER_PASS_BEGINE_OP_LOAD, "color must clear or load.");
			RequireStack(hasDepth ? (node.Depth == RENDER_PASS_BEGINE_OP_CLEAR || node.Depth == RENDER_PASS_BEGINE_OP_LOAD) :
				node.Depth == RENDER_PASS_BEGINE_OP_NONE, "invalid depth operation for the target.");
			ResolveViewport(node.Viewport,width,height);
		}
		const auto root = nodes.find(BaseCamera);
		RequireStack(root != nodes.end(), "base camera is missing.");
		const auto& base = Nodes[root->second];
		RequireStack(base.Role == CameraStackRole::Base && base.Color == RENDER_PASS_BEGINE_OP_CLEAR &&
			(!hasDepth || base.Depth == RENDER_PASS_BEGINE_OP_CLEAR), "base must clear existing attachments.");
		std::vector<uint8_t> states(Nodes.size());
		std::vector<size_t> order;
		std::function<void(size_t)> visit = [&](size_t index)
		{
			RequireStack(states[index] != 1, "cycle in overlay references.");
			RequireStack(states[index] == 0, "overlay is referenced more than once.");
			states[index] = 1; order.push_back(index);
			for (const auto& id : Nodes[index].Overlays)
			{
				const auto next = nodes.find(id);
				RequireStack(next != nodes.end(), "missing overlay camera: " + id);
				RequireStack(Nodes[next->second].Role == CameraStackRole::Overlay, "only overlays can be children.");
				visit(next->second);
			}
			states[index] = 2;
		};
		visit(root->second);
		RequireStack(order.size() == Nodes.size(), "disconnected cameras or extra base cameras.");
		return order;
	}
	CameraStackPass::CameraStackPass(std::string name, CameraStackSpecification stack, Ref<const RenderPipelineAssetContext> context,
		CameraStackBindings bindings, CameraStackRecorder record)
		: m_Name(std::move(name)), m_Stack(std::move(stack)), m_Context(std::move(context)), m_Bindings(std::move(bindings)), m_Record(std::move(record))
	{
		RequireStack(m_Context && m_Bindings && m_Record, "context, bindings and recorder are required.");
		const auto& target = m_Context->GetAsset().GetTarget(m_Stack.Target);
		m_Order = m_Stack.Validate(target.Width,target.Height,target.Samples,target.DepthStencilFormat != FRAME_BUFFER_TEXTURE_FORMAT_NONE);
		RequireStack(m_Order.size() + 1 <= Graphics::GetCommandBufferCount(), "stack exceeds graphics command capacity.");
	}
	void CameraStackPass::Configure(RenderPipelineBuilder& builder)
	{
		auto& graph = builder.GetGraph();
		const auto target = m_Context->GetTarget(m_Stack.Target);
		RenderGraph::PassHandle previous = RenderGraph::InvalidPass;
		for (auto index : m_Order)
		{
			const auto pass = graph.BuildGraphicsCopyPass(m_Name + "." + m_Stack.Nodes[index].Camera,
				[this,index](const Ref<CommandBuffer>& cmd){ Record(index,cmd); }).GetHandle();
			if (previous != RenderGraph::InvalidPass) graph.AddDependency(pass,previous);
			previous = pass;
			const auto& colors = m_Context->GetAsset().GetTarget(m_Stack.Target).ColorFormats;
			for (uint32_t i = 0; i < colors.size(); ++i)
				graph.Write(pass,graph.GetColorAttachment(target,i),RenderGraph::ResourceState::RenderTarget,GraphicsPipelineStage::ColorOutput);
		}
	}
	void CameraStackPass::Prepare(const RenderGraph& graph)
	{
		m_FrameBuffer = graph.GetFrameBuffer(m_Context->GetTarget(m_Stack.Target));
		m_RenderPasses.resize(m_Stack.Nodes.size());
		for (size_t i = 0; i < m_Stack.Nodes.size(); ++i)
		{
			auto spec = m_FrameBuffer->GetRenderPass()->GetSpecification();
			spec.Operation.ColorBegin = m_Stack.Nodes[i].Color;
			spec.Operation.DepthStencilBegin = m_Stack.Nodes[i].Depth;
			m_RenderPasses[i] = Graphics::GetRenderDevice().CreateRenderPass(spec);
		}
	}
	void CameraStackPass::Execute(RenderPipelineContext& context)
	{
		const auto bindings = m_Bindings(context);
		std::map<std::string,CameraStackBinding> byName;
		for (const auto& binding : bindings)
			RequireStack(!binding.Camera.empty() && byName.emplace(binding.Camera,binding).second, "duplicate or empty camera binding.");
		m_Views.assign(m_Stack.Nodes.size(),{});
		std::function<void(size_t,bool)> resolve = [&](size_t index,bool parentActive)
		{
			const auto& node = m_Stack.Nodes[index];
			auto& view = m_Views[index];
			view.Active = parentActive && node.Enabled;
			if (view.Active)
			{
				const auto found = byName.find(node.Camera);
				RequireStack(found != byName.end(), "missing active camera binding: " + node.Camera);
				view.Active = found->second.Active;
				if (view.Active) { RequireStack(found->second.Component != nullptr, "null active camera binding."); view.Camera = found->second.Component; }
			}
			view.Viewport = m_Stack.ResolveViewport(node.Viewport,m_FrameBuffer->GetSpecification().Width,m_FrameBuffer->GetSpecification().Height);
			for (const auto& child : node.Overlays)
				for (size_t i = 0; i < m_Stack.Nodes.size(); ++i) if (m_Stack.Nodes[i].Camera == child) resolve(i,view.Active);
		};
		resolve(m_Order.front(),true);
	}
	void CameraStackPass::Record(size_t index,const Ref<CommandBuffer>& command)
	{
		const auto& view = m_Views.at(index);
		if (!view.Active && index != m_Order.front()) return;
		command->BeginRenderPass(m_FrameBuffer,m_RenderPasses[index]);
		if (view.Active)
		{
			command->SetViewport(view.Viewport);
			m_Record(command,*view.Camera,m_Stack.Nodes[index]);
		}
		command->EndRenderPass();
	}
}

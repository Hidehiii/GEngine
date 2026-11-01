#include "CameraStackChecks.h"
#include <GEngine/Renderer/RenderPipelineAsset.h>
#include <GEngine/Graphics/GraphicsPresent.h>
#include <GEngine/Graphics/Sampler.h>
#include <GEngine/Graphics/Texture.h>
#include <array>
#include <cmath>
#include <limits>
#include <map>

namespace GEngine
{
	namespace
	{
		void Check(bool valid,const std::string& message) { if (!valid) throw std::runtime_error("Camera stack check: " + message); }
		struct CameraDraws
		{
			std::map<std::string,Ref<Material>> Materials;
			std::map<std::string,Ref<GraphicsPipeline>> Pipelines;
		};
		class StackPresent final : public RenderPipelinePass
		{
		public:
			StackPresent(std::string name,Ref<const RenderPipelineAssetContext> context) : m_Name(std::move(name)),m_Context(std::move(context)) {}
			const std::string& GetName() const override { return m_Name; }
			void Configure(RenderPipelineBuilder& builder) override
			{
				m_Target = m_Context->GetTarget("Output");
				auto& graph = builder.GetGraph();
				m_Presented = m_Context->GetTarget("Presented");
				const auto reader = graph.BuildGraphicsPass("ComposedOutput",m_Presented,[this](const Ref<CommandBuffer>& cmd){cmd->Render(m_CopyPipeline,0);}).GetHandle();
				graph.Read(reader,graph.GetColorAttachment(m_Target),RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
			}
			void Prepare(const RenderGraph& graph) override
			{
				m_CopyPipeline = CreateQuad(graph,m_Target,1.0f);
				m_Pipeline = CreateQuad(graph,m_Presented,0.8f);
			}
			void Execute(RenderPipelineContext&) override {}
			void Present(RenderPipelineContext&) override { GraphicsPresent::Render(m_Pipeline,0); }
		private:
			Ref<GraphicsPipeline> CreateQuad(const RenderGraph& graph,RenderGraph::TargetHandle source,float extent)
			{
				auto& device = Graphics::GetRenderDevice();
				auto material = device.CreateMaterial(Shader::Create(m_Context->GetAsset().GetResource("PresentShader").Path.u8string()),m_Name);
				material->SetConstant<Matrix4x4>("GE_MATRIX_M",Matrix4x4(1));
				material->SetConstant<Vector4>("GE_IMAGE_COLOR",Vector4{1,1,1,1});
				material->SetConstant<Vector2>("GE_IMAGE_TILING",Vector2{1,1});
				material->SetResource<Texture2D>("GE_IMAGE_TEXTURE",graph.GetTexture2D(graph.GetColorAttachment(source)));
				material->SetResource<Sampler>("GE_IMAGE_SAMPLER",Sampler::GetDefaultSampler());
				const float top = Graphics::GetCapabilities().AttachmentUvStartsAtTop ? 0.0f : 1.0f;
				const float bottom = 1.0f-top;
				const float vertices[][5] = {{-extent,-extent,0,0,bottom},{extent,-extent,0,1,bottom},{extent,extent,0,1,top},{-extent,extent,0,0,top}};
				const uint32_t indices[] = {0,1,2,2,3,0};
				auto buffer = device.CreateVertexBuffer(vertices,sizeof(vertices),0,VERTEX_TOPOLOGY_TRIANGLE);
				buffer->SetIndexBuffer(device.CreateIndexBuffer(indices,6));
				return device.CreateGraphicsPipeline(material,buffer);
			}
			std::string m_Name;
			Ref<const RenderPipelineAssetContext> m_Context;
			RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
			RenderGraph::TargetHandle m_Presented = RenderGraph::InvalidTarget;
			Ref<GraphicsPipeline> m_CopyPipeline;
			Ref<GraphicsPipeline> m_Pipeline;
		};
		class Checks final : public PipelineAssetChecks
		{
		public:
			Checks()
			{
				m_Template = RenderPipelineAsset::Load("Assets/Pipelines/CameraStack.gasset");
				m_Template.Save("CameraStackRoundtrip.gasset");
				Check(RenderPipelineAsset::Load("CameraStackRoundtrip.gasset").SerializePayload() == m_Template.SerializePayload(),"v2 round-trip mismatch.");
				m_Registry.Register("Example.CameraStack.v1",[](const auto& pass,const auto& asset,const auto& caps)
				{
					Check(caps.UniformBuffer && pass.Settings.size() == 1,"unsupported stack pass capabilities/settings.");
					const auto& stack = asset.GetCameraStack(pass.Settings.at("stack"));
					Check(stack.Nodes.size() == 3 && stack.BaseCamera == "Base","unsupported example stack IDs.");
				},[this](const auto& pass,const auto& context)->Ref<RenderPipelinePass>
				{
					const auto& stack = context->GetAsset().GetCameraStack(pass.Settings.at("stack"));
					auto draws = CreateRef<CameraDraws>();
					auto& device = Graphics::GetRenderDevice();
					const auto shader = Shader::Create(context->GetAsset().GetResource("CameraShader").Path.u8string());
					for (const auto& node : stack.Nodes)
					{
						auto material = device.CreateMaterial(shader,node.Camera);
						const bool ui = node.Camera == "UI";
						material->SetConstant<Vector4>("Color",node.Camera == "Base" ? Vector4{1,0,0,1} : ui ? Vector4{0,1,0,1} : Vector4{0,0,1,1});
						material->SetDepthTestOp(ui ? COMPARE_OP_ALWAYS : COMPARE_OP_LESS,0);
						material->SetEnableDepthWrite(!ui,0);
						const float z = node.Camera == "Base" ? -0.2f : -0.8f;
						const float vertices[] = {-8,-8,z, 16,-8,z, -8,16,z};
						draws->Materials[node.Camera] = material;
						draws->Pipelines[node.Camera] = device.CreateGraphicsPipeline(material,device.CreateVertexBuffer(vertices,sizeof(vertices),0,VERTEX_TOPOLOGY_TRIANGLE));
					}
					return CreateRef<CameraStackPass>(pass.Name,stack,context,
						[this,context](RenderPipelineContext& frame)
						{
							Check(frame.Cameras.size() == 3,"camera ownership changed.");
							const auto& stack = context->GetAsset().GetCameraStack("Main");
							const auto& target = context->GetAsset().GetTarget(stack.Target);
							std::vector<CameraStackBinding> bindings;
							const char* ids[] = {"Base","Scene","UI"};
							for (size_t i = 0; i < 3; ++i)
							{
								if ((m_Fault == 1 && i == 2) || (m_Case == 2 && i == 2) || (m_Case == 5 && i != 0) || (m_Case == 6 && i == 2)) continue;
								const auto rect = stack.ResolveViewport(stack.Nodes[i].Viewport,target.Width,target.Height);
								frame.Cameras[i].SetViewportSize(rect.Width,rect.Height);
								bindings.push_back({ids[i],m_Fault == 2 && i == 2 ? nullptr : &frame.Cameras[i],!((m_Case == 5 && i == 0) || (m_Case == 6 && i == 1) || (m_LiveUIOff && i == 2))});
							}
							if (m_Fault == 3) bindings.push_back(bindings.front());
							return bindings;
						},[this,draws](const Ref<CommandBuffer>& cmd,CameraComponent& camera,const CameraStackNode& node)
						{
							m_Trace.push_back(node.Camera);
							draws->Materials.at(node.Camera)->SetConstant<Matrix4x4>("Projection",camera.GetProjectionMatrix());
							if (!m_CheckedViewport)
							{
								const auto original = cmd->GetViewport();
								for (GraphicsViewport bad : {GraphicsViewport{0,0,0,1},GraphicsViewport{UINT32_MAX,0,1,1},GraphicsViewport{0,0,UINT32_MAX,1}})
								{
									bool rejected = false;
									try {cmd->SetViewport(bad);} catch(const std::invalid_argument&){rejected=true;}
									Check(rejected,"invalid viewport was accepted.");
								}
								cmd->SetViewport(original); m_CheckedViewport = true;
							}
							cmd->Render(draws->Pipelines.at(node.Camera),0);
						});
				});
				m_Registry.Register("Example.StackPresent.v1",[](const auto&,const auto& asset,const auto& caps)
				{Check(caps.Sampler && asset.GetTarget("Output").ColorFormats == std::vector<FrameBufferTextureFormat>{FRAME_BUFFER_TEXTURE_FORMAT_RGBA8},"unsupported present target.");},
				[](const auto& pass,const auto& context)->Ref<RenderPipelinePass>{return CreateRef<StackPresent>(pass.Name,context);});
				m_Selector.Activate(m_Template,m_Registry);
				Preflight();
			}
			void Execute(uint64_t frame,std::vector<CameraComponent>& cameras) override
			{
				m_LiveUIOff = frame == 1;
				if (frame == 0) m_Initial = m_Selector.GetActive().get();
				if (frame == 1 || frame == 2) Check(m_Initial == m_Selector.GetActive().get(),"live activity rebuilt the pipeline.");
				if (frame == 0)
				{
					for (int fault = 1; fault <= 3; ++fault)
					{
						m_Fault = fault;
						auto candidate = m_Registry.Create(m_Template); candidate->Prepare();
						bool rejected = false;
						try {candidate->Execute(GraphicsPresent::GetCommandBuffer(),cameras);} catch(const std::invalid_argument&){rejected=true;}
						Check(rejected && m_Trace.empty(),"invalid binding recorded GPU work.");
						GE_INFO("Camera stack binding rejection passed: {}.",fault);
					}
					m_Fault = 0;
				}
				if (frame && frame % 15 == 0)
				{
					m_Case = uint32_t(frame / 15 % 8);
					auto asset = m_Template;
					auto& stack = asset.CameraStacks[0];
					if (m_Case == 1) stack.Nodes[1].Depth = RENDER_PASS_BEGINE_OP_CLEAR;
					if (m_Case == 2) stack.Nodes[2].Enabled = false;
					if (m_Case == 3) {stack.Nodes[0].Overlays = {"UI","Scene"}; stack.Nodes[1].Depth = RENDER_PASS_BEGINE_OP_CLEAR;}
					if (m_Case == 4) {stack.Nodes[1].Color = stack.Nodes[1].Depth = RENDER_PASS_BEGINE_OP_CLEAR; stack.Nodes[1].Viewport={0,0.5f,0.5f,0.5f};}
					if (m_Case == 6) {stack.Nodes[0].Overlays={"Scene"};stack.Nodes[1].Overlays={"UI"};}
					if (m_Case == 3 || m_Case == 4 || m_Case == 7) {asset.Targets[0].Samples=4;asset.Targets[0].DepthStencilFormat=FRAME_BUFFER_TEXTURE_FORMAT_DEPTH;}
					if (m_Case == 7) {for (auto& target : asset.Targets) {target.Width=138;target.Height=86;}}
					std::weak_ptr<const RenderPipelineAssetInstance> previous = m_Selector.GetActive();
					m_Selector.Activate(asset,m_Registry);
					Check(previous.expired(),"old camera stack CPU pipeline was retained.");
					GE_INFO("Camera stack replacement passed at frame {}, case={}",frame,m_Case);
				}
				m_Trace.clear();
				m_Selector.Execute(GraphicsPresent::GetCommandBuffer(),cameras);
				std::vector<std::string> expected = {"Base","Scene","UI"};
				if (m_Case == 2 || m_LiveUIOff) expected={"Base","Scene"};
				if (m_Case == 3) expected={"Base","UI","Scene"};
				if (m_Case == 5) expected.clear();
				if (m_Case == 6) expected={"Base"};
				Check(m_Trace==expected,"camera traversal/disabled-parent order mismatch.");
				if (frame % 15 == 0 || frame == 119 || frame == 1 || frame == 2) Pixels();
				if (frame == 1 || frame == 2) GE_INFO("Camera stack live binding passed at frame {}.",frame);
			}
			void Present(std::vector<CameraComponent>& cameras) override {m_Selector.Present(GraphicsPresent::GetCommandBuffer(),cameras);}
		private:
			void Pixels()
			{
				const auto active = m_Selector.GetActive();
				const auto& target = active->GetAsset().GetTarget("Output");
				auto sourceTexture = active->GetGraph().GetTexture2D(active->GetGraph().GetColorAttachment(active->GetTarget("Output")));
				auto sampledTexture = active->GetGraph().GetTexture2D(active->GetGraph().GetColorAttachment(active->GetTarget("Presented")));
				auto& device = Graphics::GetRenderDevice();
				auto array = device.CreateTexture2DArray(target.Width,target.Height,2,RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
				RenderGraph probe;
				const auto source = probe.ImportTexture("StackOutput",sourceTexture,RenderGraph::ResourceState::ShaderRead);
				const auto sampled = probe.ImportTexture("SampledOutput",sampledTexture,RenderGraph::ResourceState::ShaderRead);
				const auto dest = probe.ImportTexture("StackReadback",array,RenderGraph::ResourceState::ShaderRead);
				const auto copy = probe.BuildTransferCopyPass("CopyStack",[&](const Ref<CommandBuffer>& cmd){device.CopyTexture2DToArrayLayer(cmd,sourceTexture,array,0);device.CopyTexture2DToArrayLayer(cmd,sampledTexture,array,1);}).GetHandle();
				probe.Read(copy,source,RenderGraph::ResourceState::CopySource,GraphicsPipelineStage::Transfer);
				probe.Read(copy,sampled,RenderGraph::ResourceState::CopySource,GraphicsPipelineStage::Transfer);
				probe.Write(copy,dest,RenderGraph::ResourceState::CopyDestination,GraphicsPipelineStage::Transfer);
				const auto join = probe.BuildGraphicsCopyPass("StackReadable",[](const Ref<CommandBuffer>&){}).GetHandle();
				probe.Read(join,source,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
				probe.Read(join,sampled,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
				probe.Read(join,dest,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
				probe.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
				std::vector<uint8_t> pixels(target.Width*target.Height*4);
				for (uint32_t layer=0;layer<2;++layer)
				{
				array->ReadLayerData(layer,uint32_t(pixels.size()),pixels.data());
				for (uint32_t quadrant=0;quadrant<4;++quadrant)
				{
					const uint32_t x=(quadrant%2 ? 3 : 1)*target.Width/4;
					uint32_t y=(quadrant/2 ? 3 : 1)*target.Height/4;
					// The diagnostic array preserves native attachment row order.
					if (!Graphics::GetCapabilities().AttachmentUvStartsAtTop) y=target.Height-1-y;
					std::array<int,4> color={255,0,0,255};
					if (m_Case==1 || m_Case==3) color={0,0,255,255};
					if (m_Case==4 || m_Case==5) color={0,0,0,0};
					if (m_Case==4 && quadrant==2) color={0,0,255,255};
					if (!m_LiveUIOff && (m_Case==0 || m_Case==1 || m_Case==4 || m_Case==7) && quadrant==1) color={0,255,0,255};
					const auto offset=(y*target.Width+x)*4;
					for (size_t channel=0;channel<4;++channel)
						Check(std::abs(int(pixels[offset+channel])-color[channel])<=2,"pixel mismatch layer="+std::to_string(layer)+" case="+std::to_string(m_Case)+" quadrant="+std::to_string(quadrant)+" channel="+std::to_string(channel)+" actual="+std::to_string(pixels[offset+channel]));
				}
				}
				GE_INFO("Camera stack pixels passed: case={}, {}x{}, samples={}.",m_Case,target.Width,target.Height,target.Samples);
			}
			void Preflight()
			{
				auto original=m_Selector.GetActive(); uint32_t count=0;
				auto reject=[&](RenderPipelineAsset asset)
				{
					bool rejected=false;
					try{m_Selector.Activate(asset,m_Registry);}catch(const std::exception&){rejected=true;}
					Check(rejected && m_Selector.GetActive()==original,"bad stack did not preserve selection."); ++count;
				};
				auto asset=m_Template; asset.Version=1; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes.push_back(asset.CameraStacks[0].Nodes[1]); reject(asset);
				asset=m_Template; asset.CameraStacks[0].BaseCamera="Missing"; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[1].Role=CameraStackRole::Base; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[0].Color=RENDER_PASS_BEGINE_OP_LOAD; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[0].Overlays.push_back("Missing"); reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[0].Overlays.push_back("UI"); reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[1].Overlays={"UI"}; asset.CameraStacks[0].Nodes[2].Overlays={"Scene"}; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[0].Overlays={"Scene"}; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[1].Target="Other"; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[1].Samples=4; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[2].Viewport.X=0.75f; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[2].Viewport.Width=1e-9f; reject(asset);
				asset=m_Template; asset.CameraStacks[0].Nodes[2].Viewport.X=std::numeric_limits<float>::quiet_NaN(); reject(asset);
				asset=m_Template; asset.Targets[0].DepthStencilFormat=FRAME_BUFFER_TEXTURE_FORMAT_NONE; reject(asset);
				asset=m_Template; asset.Passes[0].Settings["stack"]="Missing"; reject(asset);
				GE_INFO("Camera stack preflight passed: {} cases.",count);
			}
			RenderPipelineAsset m_Template;
			RenderPipelineAssetRegistry m_Registry;
			RenderPipelineSelector m_Selector;
			std::vector<std::string> m_Trace;
			uint32_t m_Case=0;
			int m_Fault=0;
			bool m_CheckedViewport=false;
			bool m_LiveUIOff=false;
			const RenderPipelineAssetInstance* m_Initial=nullptr;
		};
	}
	Ref<PipelineAssetChecks> CreateCameraStackChecks(){return CreateRef<Checks>();}
}

#include "PostProcessChecks.h"
#include <GEngine/Renderer/RenderPipelineAsset.h>
#include <GEngine/Graphics/GraphicsPresent.h>
#include <GEngine/Graphics/Sampler.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>

namespace GEngine
{
	namespace
	{
		void CheckPost(bool valid, const std::string& message)
		{
			if (!valid) throw std::runtime_error("Post-process check: " + message);
		}
		class PostPresent final : public RenderPipelinePass
		{
		public:
			PostPresent(std::string name, Ref<const RenderPipelineAssetContext> context) : m_Name(std::move(name)), m_Context(std::move(context)) {}
			const std::string& GetName() const override { return m_Name; }
			void Configure(RenderPipelineBuilder& builder) override
			{
				auto& graph = builder.GetGraph();
				const auto pass = graph.BuildGraphicsCopyPass("PostReadable",[](const Ref<CommandBuffer>&){}).GetHandle();
				graph.Read(pass,graph.GetColorAttachment(m_Context->GetTarget("Output")),RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
			}
			void Prepare(const RenderGraph& graph) override
			{
				auto& device = Graphics::GetRenderDevice();
				auto material = device.CreateMaterial(Shader::Create(m_Context->GetAsset().GetResource("PresentShader").Path.u8string()),m_Name);
				material->SetConstant<Matrix4x4>("GE_MATRIX_M",Matrix4x4(1));
				material->SetConstant<Vector4>("GE_IMAGE_COLOR",Vector4{1,1,1,1});
				material->SetConstant<Vector2>("GE_IMAGE_TILING",Vector2{1,1});
				material->SetResource<Texture2D>("GE_IMAGE_TEXTURE",graph.GetTexture2D(graph.GetColorAttachment(m_Context->GetTarget("Output"))));
				material->SetResource<Sampler>("GE_IMAGE_SAMPLER",Sampler::GetDefaultSampler());
				const float top = Graphics::GetCapabilities().AttachmentUvStartsAtTop ? 0.0f : 1.0f;
				const float vertices[][5] = {{-.8f,-.8f,0,0,1-top},{.8f,-.8f,0,1,1-top},{.8f,.8f,0,1,top},{-.8f,.8f,0,0,top}};
				const uint32_t indices[] = {0,1,2,2,3,0};
				auto buffer = device.CreateVertexBuffer(vertices,sizeof(vertices),0,VERTEX_TOPOLOGY_TRIANGLE);
				buffer->SetIndexBuffer(device.CreateIndexBuffer(indices,6));
				m_Pipeline = device.CreateGraphicsPipeline(material,buffer);
			}
			void Execute(RenderPipelineContext&) override {}
			void Present(RenderPipelineContext&) override { GraphicsPresent::Render(m_Pipeline,0); }
		private:
			std::string m_Name;
			Ref<const RenderPipelineAssetContext> m_Context;
			Ref<GraphicsPipeline> m_Pipeline;
		};
		class PostChecks final : public PipelineAssetChecks
		{
		public:
			PostChecks()
			{
				CheckPost(Graphics::GetCommandBufferCount() >= 9,"diagnostic mode needs CommandBufferCount >= 9 (runner uses 12).");
				m_Template = RenderPipelineAsset::Load("Assets/Pipelines/PostProcess.gasset");
				m_Template.Save("PostProcessRoundtrip.gasset");
				CheckPost(RenderPipelineAsset::Load("PostProcessRoundtrip.gasset").SerializePayload() == m_Template.SerializePayload(),"v3 round-trip mismatch.");
				m_Registry.Register("Example.HDRCameras.v1",[](const auto&,const auto& asset,const auto&)
				{
					CheckPost(asset.GetCameraStack("Main").Nodes.size() == 2,"expected two HDR cameras.");
				},[this](const auto& pass,const auto& context)->Ref<RenderPipelinePass>
				{
					auto& device = Graphics::GetRenderDevice();
					auto shader = Shader::Create(context->GetAsset().GetResource("CameraShader").Path.u8string());
					std::array<Ref<Material>,2> materials;
					std::array<Ref<GraphicsPipeline>,2> pipelines;
					for (size_t i = 0; i < 2; ++i)
					{
						materials[i] = device.CreateMaterial(shader,i ? "HDR UI" : "HDR Base");
						materials[i]->SetConstant<Vector4>("Color",i ? Vector4{.25f,2,.002f,.75f} : Vector4{4,.5f,.125f,.5f});
						materials[i]->SetDepthTestOp(COMPARE_OP_ALWAYS,0);
						materials[i]->SetEnableDepthWrite(false,0);
						const float vertices[] = {-8,-8,-.2f,16,-8,-.2f,-8,16,-.2f};
						pipelines[i] = device.CreateGraphicsPipeline(materials[i],device.CreateVertexBuffer(vertices,sizeof(vertices),0,VERTEX_TOPOLOGY_TRIANGLE));
					}
					return CreateRef<CameraStackPass>(pass.Name,context->GetAsset().GetCameraStack("Main"),context,
						[context](RenderPipelineContext& frame)
						{
							CheckPost(frame.Cameras.size() == 2,"camera ownership changed.");
							const auto& target = context->GetAsset().GetTarget("HDR");
							frame.Cameras[0].SetViewportSize(target.Width,target.Height);
							frame.Cameras[1].SetViewportSize(target.Width/2,target.Height/2);
							return std::vector<CameraStackBinding>{{"Base",&frame.Cameras[0],true},{"UI",&frame.Cameras[1],true}};
						},[this,materials,pipelines](const Ref<CommandBuffer>& command,CameraComponent& camera,const CameraStackNode& node) mutable
						{
							const size_t index = node.Camera == "Base" ? 0 : 1;
							m_Trace.push_back(node.Camera);
							materials[index]->SetConstant<Matrix4x4>("Projection",camera.GetProjectionMatrix());
							command->Render(pipelines[index],0);
						});
				});
				RegisterPostProcessPass(m_Registry,[this](RenderPipelineContext&)
				{
					auto settings = m_Current;
					if (m_Fault == 1) settings.Effects[0].EV = std::numeric_limits<float>::quiet_NaN();
					if (m_Fault == 2) settings.Effects.pop_back();
					if (m_Fault == 3) settings.Encoding = PostProcessEncoding(99);
					if (m_Fault == 4) settings.Effects[0].ID = "Unknown";
					return settings;
				});
				m_Registry.Register("Example.PostPresent.v1",[](const auto&,const auto&,const auto&){},
					[](const auto& pass,const auto& context)->Ref<RenderPipelinePass>{return CreateRef<PostPresent>(pass.Name,context);});
				m_Selector.Activate(m_Template,m_Registry);
				m_Current = Settings(0);
				Preflight();
			}
			void Execute(uint64_t frame,std::vector<CameraComponent>& cameras) override
			{
				m_Case = uint32_t(frame/15%8);
				if (frame == 0)
				{
					for (int fault = 1; fault <= 4; ++fault)
					{
						m_Fault = fault;
						auto candidate = m_Registry.Create(m_Template); candidate->Prepare();
						bool rejected = false;
						try {candidate->Execute(GraphicsPresent::GetCommandBuffer(),cameras);} catch(const std::invalid_argument&){rejected=true;}
						CheckPost(rejected && m_Trace.empty(),"invalid live settings recorded GPU work.");
						GE_INFO("Post-process live rejection passed: {}.",fault);
					}
					m_Fault = 0;
				}
				if (frame && frame%15 == 0 && m_Case != 5)
				{
					auto asset = m_Template;
					if (m_Case >= 4) for (auto& target : asset.Targets) {target.Width=138;target.Height=86;}
					asset.PostProcessStacks[0].Settings = Settings(m_Case);
					std::weak_ptr<const RenderPipelineAssetInstance> previous = m_Selector.GetActive();
					m_Selector.Activate(asset,m_Registry);
					CheckPost(previous.expired(),"outgoing CPU pipeline retained.");
					m_Targets.clear();
					GE_INFO("Post-process replacement passed at frame {}.",frame);
				}
				m_Current = Settings(m_Case);
				if (frame == 1) {m_Current.Enabled=true;m_Current.Effects[0].EV=16;}
				if (frame == 0) m_Initial = m_Selector.GetActive().get();
				if (frame == 1 || frame == 2) CheckPost(m_Initial == m_Selector.GetActive().get(),"live settings replaced the pipeline.");
				m_Trace.clear();
				m_Selector.Execute(GraphicsPresent::GetCommandBuffer(),cameras);
				CheckPost(m_Trace == std::vector<std::string>{"Base","UI"},"effects did not consume the completed camera stack.");
				CheckTargets();
				if (frame%15 == 0 || frame == 1 || frame == 2 || frame == 119) Pixels();
				if (frame == 1 || frame == 2) GE_INFO("Post-process live settings passed at frame {}.",frame);
			}
			void Present(std::vector<CameraComponent>& cameras) override {m_Selector.Present(GraphicsPresent::GetCommandBuffer(),cameras);}
		private:
			PostProcessSettings Settings(uint32_t mode) const
			{
				auto settings = m_Template.PostProcessStacks[0].Settings;
				if (mode == 0) settings.Enabled=false;
				if (mode == 1) {settings.Effects[0].EV=-2;settings.Effects.pop_back();}
				if (mode == 2) settings.Effects.erase(settings.Effects.begin());
				if (mode == 4) {settings.Effects[0].EV=-1;std::reverse(settings.Effects.begin(),settings.Effects.end());}
				if (mode == 5) {settings.Effects[0].EV=-16;settings.Effects[1].Enabled=false;settings.Encoding=PostProcessEncoding::SRGB;}
				if (mode == 6) settings.Effects.clear();
				if (mode == 7) {settings.Effects[0].EV=0;settings.Encoding=PostProcessEncoding::SRGB;}
				return settings;
			}
			void CheckTargets()
			{
				const auto active = m_Selector.GetActive();
				const auto count = m_Current.Effects.size() > 1 ? 3u : 2u;
				for (uint32_t i = 0; i < count; ++i)
				{
					auto target = active->GetGraph().GetFrameBuffer(i);
					if (m_Targets.size() == i) m_Targets.push_back(target);
					else CheckPost(m_Targets[i].lock() == target,"live settings allocated a new target.");
					if (i == 2) CheckPost(target->GetSpecification().RenderTargets == std::vector<FrameBufferTextureFormat>{FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F},"intermediate lost HDR precision.");
				}
				bool absent = false;
				try {active->GetGraph().GetFrameBuffer(count);} catch(const std::out_of_range&) {absent=true;}
				CheckPost(absent,"unexpected intermediate allocation count.");
			}
			void Pixels()
			{
				const auto active = m_Selector.GetActive();
				const auto& target = active->GetAsset().GetTarget("Output");
				auto texture = active->GetGraph().GetTexture2D(active->GetGraph().GetColorAttachment(active->GetTarget("Output")));
				auto& device = Graphics::GetRenderDevice();
				auto array = device.CreateTexture2DArray(target.Width,target.Height,1,RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
				RenderGraph probe;
				const auto source = probe.ImportTexture("PostOutput",texture,RenderGraph::ResourceState::ShaderRead);
				const auto destination = probe.ImportTexture("PostReadback",array,RenderGraph::ResourceState::ShaderRead);
				const auto copy = probe.BuildTransferCopyPass("CopyPost",[&](const Ref<CommandBuffer>& command){device.CopyTexture2DToArrayLayer(command,texture,array,0);}).GetHandle();
				probe.Read(copy,source,RenderGraph::ResourceState::CopySource,GraphicsPipelineStage::Transfer);
				probe.Write(copy,destination,RenderGraph::ResourceState::CopyDestination,GraphicsPipelineStage::Transfer);
				const auto join = probe.BuildGraphicsCopyPass("PostReadable",[](const Ref<CommandBuffer>&){}).GetHandle();
				probe.Read(join,source,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
				probe.Read(join,destination,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
				probe.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
				std::vector<uint8_t> pixels(target.Width*target.Height*4);
				array->ReadLayerData(0,uint32_t(pixels.size()),pixels.data());
				for (uint32_t quadrant = 0; quadrant < 4; ++quadrant)
				{
					std::array<double,4> expected = quadrant == 1 ? std::array<double,4>{.25,2,.002,.75} : std::array<double,4>{4,.5,.125,.5};
					for (size_t channel = 0; channel < 3; ++channel)
					{
						auto& value = expected[channel];
						for (const auto& effect : m_Current.Effects)
							if (m_Current.Enabled && effect.Enabled)
								value = effect.Type == PostProcessEffectType::Exposure ? std::min(65504.0,value*std::exp2(effect.EV)) : value/(1+value);
						value = std::clamp(value,0.0,1.0);
						if (m_Current.Encoding == PostProcessEncoding::SRGB) value = value <= .0031308 ? 12.92*value : 1.055*std::pow(value,1.0/2.4)-.055;
					}
					const uint32_t x = (quadrant%2 ? 3 : 1)*target.Width/4;
					uint32_t y = (quadrant/2 ? 3 : 1)*target.Height/4;
					if (!Graphics::GetCapabilities().AttachmentUvStartsAtTop) y = target.Height-1-y;
					for (size_t channel = 0; channel < 4; ++channel)
					{
						const auto actual = pixels[(y*target.Width+x)*4+channel];
						CheckPost(std::abs(actual-std::lround(expected[channel]*255)) <= 2,"pixel mismatch case="+std::to_string(m_Case)+" quadrant="+std::to_string(quadrant)+" channel="+std::to_string(channel)+" actual="+std::to_string(actual));
					}
				}
				GE_INFO("Post-process pixels passed: case={}, {}x{}, stages={}.",m_Case,target.Width,target.Height,m_Current.Effects.size());
			}
			void Preflight()
			{
				auto reusableRegistry = m_Registry;
				Ref<PostProcessStackPass> reusable;
				reusableRegistry.Register("Example.ReusedPost.v1",[](const auto&,const auto&,const auto&){},
					[&reusable](const auto& pass,const auto& context)->Ref<RenderPipelinePass>
					{
						if (!reusable) reusable = CreateRef<PostProcessStackPass>(pass.Name,context->GetAsset().GetPostProcessStack("Main"),context);
						return reusable;
					});
				auto reusableAsset = m_Template;
				reusableAsset.Passes[1].Type = "Example.ReusedPost.v1";
				for (int iteration = 0; iteration < 2; ++iteration)
				{
					auto candidate = reusableRegistry.Create(reusableAsset);
					candidate->Prepare();
					CheckPost(reusable->GetIntermediateTargets().size() == 1,"reconfiguration retained old intermediate declarations.");
				}
				GE_INFO("Post-process pass reuse passed.");
				const auto original = m_Selector.GetActive();
				uint32_t count = 0;
				auto reject = [&](const RenderPipelineAsset& asset)
				{
					bool rejected = false;
					try {m_Selector.Activate(asset,m_Registry);} catch(const std::exception&) {rejected=true;}
					CheckPost(rejected && original == m_Selector.GetActive(),"invalid stack changed selection."); ++count;
				};
				auto asset = m_Template; asset.Version=2; reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Output="HDR"; reject(asset);
				asset=m_Template; asset.Targets[0].ColorFormats={FRAME_BUFFER_TEXTURE_FORMAT_RGBA8}; reject(asset);
				asset=m_Template; asset.Targets[1].ColorFormats={FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F}; reject(asset);
				asset=m_Template; asset.Targets[1].Samples=4; reject(asset);
				asset=m_Template; asset.Targets[1].DepthStencilFormat=FRAME_BUFFER_TEXTURE_FORMAT_DEPTH; reject(asset);
				asset=m_Template; asset.Targets[1].Width=97; reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Settings.Effects[0].EV=std::numeric_limits<float>::quiet_NaN(); reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Settings.Effects[0].EV=17; reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Settings.Effects[1].ID="Exposure"; reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Settings.Effects[0].Type=PostProcessEffectType(99); reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Settings.Encoding=PostProcessEncoding(99); reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Settings.Effects[0].ID.clear(); reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Shader="Missing"; reject(asset);
				asset=m_Template; asset.PostProcessStacks[0].Input="Missing"; reject(asset);
				asset=m_Template; asset.Passes[1].Settings["stack"]="Missing"; reject(asset);
				asset=m_Template; asset.Resources["EffectsShader"]=asset.Resources["PresentShader"]; reject(asset);
				asset=m_Template; asset.PostProcessStacks.push_back(asset.PostProcessStacks[0]); reject(asset);
				GE_INFO("Post-process preflight passed: {} cases.",count);
			}
			RenderPipelineAsset m_Template;
			RenderPipelineAssetRegistry m_Registry;
			RenderPipelineSelector m_Selector;
			PostProcessSettings m_Current;
			std::vector<std::string> m_Trace;
			std::vector<std::weak_ptr<FrameBuffer>> m_Targets;
			const RenderPipelineAssetInstance* m_Initial = nullptr;
			uint32_t m_Case = 0;
			int m_Fault = 0;
		};
	}
	Ref<PipelineAssetChecks> CreatePostProcessChecks() {return CreateRef<PostChecks>();}
}

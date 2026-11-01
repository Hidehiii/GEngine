#include "PipelineAssetChecks.h"
#include <GEngine/Renderer/RenderPipelineAsset.h>
#include <GEngine/Graphics/FrameBuffer.h>
#include <GEngine/Graphics/GraphicsPresent.h>
#include <GEngine/Graphics/Sampler.h>
#include <GEngine/Graphics/Texture.h>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace GEngine
{
	namespace
	{
		void Check(bool ok, const std::string& message)
		{
			if (!ok) throw std::runtime_error("Pipeline asset check: " + message);
		}
		template<typename Callback> void Reject(Callback&& callback)
		{
			try { callback(); } catch (const std::logic_error&) { return; }
			throw std::runtime_error("Pipeline selection accepted a frame-boundary violation.");
		}
		const std::string& Setting(const RenderPipelinePassAsset& pass, const std::string& key)
		{
			const auto found = pass.Settings.find(key);
			if (found == pass.Settings.end()) throw std::invalid_argument("Missing pipeline pass setting: " + key);
			return found->second;
		}
		float Channel(const RenderPipelinePassAsset& pass, const std::string& key)
		{
			const auto& text = Setting(pass, key);
			size_t used = 0;
			const float value = std::stof(text, &used);
			if (used != text.size() || !std::isfinite(value) || value < 0 || value > 1)
				throw std::invalid_argument("Invalid finite color channel: " + key);
			return value;
		}
		struct ProbeState
		{
			RenderPipelineSelector* Selector = nullptr;
			RenderPipelineAssetRegistry* Registry = nullptr;
			std::vector<CameraComponent>* Cameras = nullptr;
			uint32_t Factories = 0;
			void Boundary() const
			{
				Reject([&] { Selector->Clear(); });
				Reject([&] { Selector->Activate(std::filesystem::path("MustNotLoad.gasset"), *Registry); });
			}
		};
		class SolidPass final : public RenderPipelinePass
		{
		public:
			SolidPass(RenderPipelinePassAsset pass, Ref<const RenderPipelineAssetContext> context, Ref<ProbeState> probe)
				: m_Pass(std::move(pass)), m_Context(std::move(context)), m_Probe(std::move(probe)) {}
			const std::string& GetName() const override { return m_Pass.Name; }
			void Configure(RenderPipelineBuilder& builder) override
			{
				m_Probe->Boundary();
				const auto path = m_Context->GetAsset().GetResource(Setting(m_Pass, "shader")).Path;
				auto& device = Graphics::GetRenderDevice();
				auto material = device.CreateMaterial(Shader::Create(path.u8string()), m_Pass.Name);
				material->SetConstant<Vector4>("OutputColor", Vector4{ Channel(m_Pass,"red"), Channel(m_Pass,"green"), Channel(m_Pass,"blue"), 1 });
				const float vertices[] = { -1,-1, 3,-1, -1,3 };
				m_Pipeline = device.CreateGraphicsPipeline(material, device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE));
				builder.AddGraphicsPass(m_Pass.Name, m_Context->GetTarget(Setting(m_Pass,"target")),
					[this](const Ref<CommandBuffer>& cmd) { cmd->Render(m_Pipeline, 0); });
			}
			void Prepare(const RenderGraph&) override
			{
				Check(!m_Prepared, "pass was prepared more than once.");
				m_Probe->Boundary();
				m_Prepared = true;
			}
			void Execute(RenderPipelineContext& context) override
			{
				Check(m_Prepared && context.Frame == m_Frame && &context.Cameras == m_Probe->Cameras, "frame/camera ownership changed.");
				m_Probe->Boundary();
			}
			void Present(RenderPipelineContext& context) override
			{
				Check(context.Frame == m_Frame, "Execute/Present frame mismatch.");
				m_Probe->Boundary();
				++m_Frame;
			}
		private:
			RenderPipelinePassAsset m_Pass;
			Ref<const RenderPipelineAssetContext> m_Context;
			Ref<ProbeState> m_Probe;
			Ref<GraphicsPipeline> m_Pipeline;
			uint64_t m_Frame = 0;
			bool m_Prepared = false;
		};
		class SamplePresentPass final : public RenderPipelinePass
		{
		public:
			SamplePresentPass(RenderPipelinePassAsset pass, Ref<const RenderPipelineAssetContext> context)
				: m_Pass(std::move(pass)), m_Context(std::move(context)) {}
			const std::string& GetName() const override { return m_Pass.Name; }
			void Configure(RenderPipelineBuilder& builder) override
			{
				m_Target = m_Context->GetTarget(Setting(m_Pass,"target"));
				auto& graph = builder.GetGraph();
				const auto reader = graph.BuildGraphicsCopyPass(m_Pass.Name + ".Sampled", [](const Ref<CommandBuffer>&) {}).GetHandle();
				graph.Read(reader, graph.GetColorAttachment(m_Target), RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
			}
			void Prepare(const RenderGraph& graph) override
			{
				const auto texture = graph.GetTexture2D(graph.GetColorAttachment(m_Target));
				Check(texture != nullptr, "target missing during Prepare.");
				auto& device = Graphics::GetRenderDevice();
				const auto path = m_Context->GetAsset().GetResource(Setting(m_Pass,"shader")).Path;
				auto material = device.CreateMaterial(Shader::Create(path.u8string()), m_Pass.Name);
				material->SetConstant<Matrix4x4>("GE_MATRIX_M", Matrix4x4(1));
				material->SetConstant<Vector4>("GE_IMAGE_COLOR", Vector4{1,1,1,1});
				material->SetConstant<Vector2>("GE_IMAGE_TILING", Vector2{1,1});
				material->SetResource<Texture2D>("GE_IMAGE_TEXTURE", texture);
				material->SetResource<Sampler>("GE_IMAGE_SAMPLER", Sampler::GetDefaultSampler());
				const float vertices[][5] = { {-0.5f,-0.5f,0,0,0}, {0.5f,-0.5f,0,1,0}, {0.5f,0.5f,0,1,1}, {-0.5f,0.5f,0,0,1} };
				const uint32_t indices[] = {0,1,2,2,3,0};
				auto buffer = device.CreateVertexBuffer(vertices, sizeof(vertices), 0, VERTEX_TOPOLOGY_TRIANGLE);
				buffer->SetIndexBuffer(device.CreateIndexBuffer(indices, 6));
				m_Pipeline = device.CreateGraphicsPipeline(material, buffer);
			}
			void Execute(RenderPipelineContext&) override {}
			void Present(RenderPipelineContext&) override
			{
				Check(m_Pipeline != nullptr, "presentation pipeline was not prepared before activation.");
				GraphicsPresent::Render(m_Pipeline, 0);
			}
		private:
			RenderPipelinePassAsset m_Pass;
			Ref<const RenderPipelineAssetContext> m_Context;
			RenderGraph::TargetHandle m_Target = RenderGraph::InvalidTarget;
			Ref<GraphicsPipeline> m_Pipeline;
		};
		class FaultPass final : public RenderPipelinePass
		{
		public:
			explicit FaultPass(RenderPipelinePassAsset pass) : m_Pass(std::move(pass)) {}
			const std::string& GetName() const override { return m_Pass.Name; }
			void Configure(RenderPipelineBuilder& builder) override
			{
				if (Setting(m_Pass,"phase") == "configure") throw std::runtime_error("Intentional candidate configuration failure.");
				if (Setting(m_Pass,"phase") == "cycle")
				{
					auto& graph = builder.GetGraph();
					const auto first = graph.BuildGraphicsCopyPass("CycleA", [](const Ref<CommandBuffer>&) {}).GetHandle();
					const auto second = graph.BuildGraphicsCopyPass("CycleB", [](const Ref<CommandBuffer>&) {}).GetHandle();
					graph.AddDependency(first,second); graph.AddDependency(second,first);
				}
			}
			void Prepare(const RenderGraph&) override
			{
				if (Setting(m_Pass,"phase") == "prepare") throw std::runtime_error("Intentional candidate preparation failure.");
			}
			void Execute(RenderPipelineContext&) override {}
		private:
			RenderPipelinePassAsset m_Pass;
		};
		void ValidatePass(const RenderPipelinePassAsset& pass, const RenderPipelineAsset& asset,
			const GraphicsCapabilities& caps, bool present)
		{
			if (!caps.UniformBuffer || (present && !caps.Sampler)) throw std::invalid_argument("Unsupported example pass capability.");
			Check(asset.Settings.size() == 1 && asset.Settings.count("label"), "example requires only a label setting.");
			const auto& target = asset.GetTarget(Setting(pass,"target"));
			Check(target.ColorFormats == std::vector<FrameBufferTextureFormat>{FRAME_BUFFER_TEXTURE_FORMAT_RGBA8}, "example requires one RGBA8 target.");
			Check(asset.GetResource(Setting(pass,"shader")).Type == "shader", "shader dependency has the wrong type.");
			Check(pass.Settings.size() == (present ? 2 : 5), "unknown or missing example pass settings.");
			if (!present) { Channel(pass,"red"); Channel(pass,"green"); Channel(pass,"blue"); }
			else
			{
				bool producer = false;
				for (const auto& previous : asset.Passes)
				{
					if (previous.Name == pass.Name) break;
					if (previous.Type == "Example.SolidColor.v1" && Setting(previous,"target") == target.Name) producer = true;
				}
				Check(producer, "presentation must follow its color producer.");
			}
		}

		void ReadOutput(const RenderPipelineAssetInstance& pipeline)
		{
			const auto& target = pipeline.GetAsset().GetTarget("Output");
			const auto& graph = pipeline.GetGraph();
			auto texture = graph.GetTexture2D(graph.GetColorAttachment(pipeline.GetTarget("Output")));
			Check(texture && texture->GetWidth() == target.Width && texture->GetHeight() == target.Height, "selected target dimensions mismatch.");
			auto& device = Graphics::GetRenderDevice();
			auto array = device.CreateTexture2DArray(target.Width,target.Height,1,RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
			RenderGraph probe;
			const auto source = probe.ImportTexture("SelectedColor",texture,RenderGraph::ResourceState::ShaderRead);
			const auto dest = probe.ImportTexture("SelectedReadback",array,RenderGraph::ResourceState::ShaderRead);
			auto copy = probe.BuildTransferCopyPass("SelectedCopy", [&](const Ref<CommandBuffer>& cmd)
			{ device.CopyTexture2DToArrayLayer(cmd,texture,array,0); }).GetHandle();
			probe.Read(copy,source,RenderGraph::ResourceState::CopySource,GraphicsPipelineStage::Transfer);
			probe.Write(copy,dest,RenderGraph::ResourceState::CopyDestination,GraphicsPipelineStage::Transfer);
			auto join = probe.BuildGraphicsCopyPass("SelectedReadable",[](const Ref<CommandBuffer>&){}).GetHandle();
			probe.Read(join,source,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
			probe.Read(join,dest,RenderGraph::ResourceState::ShaderRead,GraphicsPipelineStage::FragmentShader);
			probe.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
			std::vector<uint8_t> pixels(target.Width * target.Height * 4);
			array->ReadLayerData(0,static_cast<uint32_t>(pixels.size()),pixels.data());
			const auto& pass = pipeline.GetAsset().Passes.front();
			const float expected[] = {Channel(pass,"red"),Channel(pass,"green"),Channel(pass,"blue"),1};
			for (size_t pixel : {size_t(0),size_t((target.Height / 2 * target.Width + target.Width / 2) * 4)})
				for (size_t channel = 0; channel < 4; ++channel)
					Check(std::abs(static_cast<int>(pixels[pixel+channel])-static_cast<int>(expected[channel]*255+0.5f)) <= 2,"selected pipeline GPU color mismatch.");
			GE_INFO("Pipeline asset pixels passed: {} {}x{} samples={}.",pipeline.GetAsset().Settings.at("label"),target.Width,target.Height,target.Samples);
		}

		class Checks final : public PipelineAssetChecks
		{
		public:
			Checks()
			{
				m_Probe = CreateRef<ProbeState>();
				m_Probe->Selector = &m_Selector; m_Probe->Registry = &m_Registry;
				m_Registry.Register("Example.SolidColor.v1",[](const auto& p,const auto& a,const auto& c){ValidatePass(p,a,c,false);},
					[this](const auto& p,const auto& c){++m_Probe->Factories;return CreateRef<SolidPass>(p,c,m_Probe);});
				m_Registry.Register("Example.Present.v1",[](const auto& p,const auto& a,const auto& c){ValidatePass(p,a,c,true);},
					[this](const auto& p,const auto& c){++m_Probe->Factories;return CreateRef<SamplePresentPass>(p,c);});
				m_Registry.Register("Example.Fault.v1",[](const auto&,const auto&,const auto&){},
					[](const auto& p,const auto&)->Ref<RenderPipelinePass>{return CreateRef<FaultPass>(p);});
				m_Registry.Register("Example.Null.v1",[](const auto&,const auto&,const auto&){},
					[](const auto&,const auto&)->Ref<RenderPipelinePass>{return nullptr;});
				m_Warm = RenderPipelineAsset::Load("Assets/Pipelines/Warm.gasset");
				m_Cool = RenderPipelineAsset::Load("Assets/Pipelines/Cool.gasset");
				m_Warm.Save("PipelineAssetRoundtrip.gasset");
				Check(RenderPipelineAsset::Load("PipelineAssetRoundtrip.gasset").SerializePayload() == m_Warm.SerializePayload(),"GAsset round-trip mismatch.");
				Check(RenderPipelineAsset::ParsePayload(m_Cool.SerializePayload()).SerializePayload() == m_Cool.SerializePayload(),"payload round-trip mismatch.");
				auto snapshotRegistry = m_Registry;
				snapshotRegistry.Register("Example.Snapshot.v1",
					[&](const auto&,const auto&,const auto&){snapshotRegistry = RenderPipelineAssetRegistry{};},
					[](const auto& p,const auto&)->Ref<RenderPipelinePass>{return CreateRef<FaultPass>(p);});
				auto snapshotAsset = m_Warm;
				snapshotAsset.Passes.push_back({"Snapshot","Example.Snapshot.v1",{{"phase","noop"}}});
				m_Selector.Activate(snapshotAsset,snapshotRegistry);
				GE_INFO("Pipeline asset registry snapshot passed.");
				m_Selector.Activate(std::filesystem::path("Assets/Pipelines/Warm.gasset"),m_Registry);
				Failures();
				GE_INFO("Pipeline asset preflight and rollback passed.");
			}
			void Execute(uint64_t frame,std::vector<CameraComponent>& cameras) override
			{
				m_Probe->Cameras = &cameras;
				if (frame && frame % 15 == 0)
				{
					Ref<CommandBuffer> pending;
					std::weak_ptr<const RenderPipelineAssetInstance> outgoing;
					{
						auto old = m_Selector.GetActive(); outgoing = old;
						auto framebuffer = old->GetGraph().GetFrameBuffer(old->GetTarget("LifetimeProbe"));
						pending = Graphics::GetGraphicsCommandBuffer();
						pending->Begin(framebuffer); pending->End();
					}
					if ((frame / 15) % 2) m_Selector.Activate(std::filesystem::path("Assets/Pipelines/Cool.gasset"),m_Registry);
					else
					{
						auto resized = m_Warm;
						if (frame >= 60) { resized.Targets[0].Width = 137; resized.Targets[0].Height = 83; }
						m_Selector.Activate(resized,m_Registry);
						resized.Passes[0].Settings["red"] = "0";
						Check(m_Selector.GetActive()->GetAsset().Passes[0].Settings.at("red") == "0.9","active settings were not a snapshot.");
					}
					Check(outgoing.expired(),"selector retained outgoing CPU pipelines indefinitely.");
					const bool submit = (frame / 15) % 2 != 0;
					if (submit) Graphics::GetRenderDevice().GetQueue(COMMAND_BUFFER_TYPE_GRAPHICS).Submit(pending);
					else { pending->Begin(); pending->End(); }
					GE_INFO("Pipeline asset switch passed at frame {}; recorded outgoing target {}.",frame,submit ? "submitted" : "discarded/reset");
				}
				const auto command = GraphicsPresent::GetCommandBuffer();
				if (frame == 0) Reject([&]{m_Selector.Present(command,cameras);});
				m_Selector.Execute(command,cameras);
				Reject([&]{m_Selector.Execute(command,cameras);});
				Reject([&]{m_Selector.Activate(m_Cool,m_Registry);});
				Reject([&]{m_Selector.Clear();});
				if (frame % 15 == 0 || frame == 119) ReadOutput(*m_Selector.GetActive());
			}
			void Present(std::vector<CameraComponent>& cameras) override
			{
				const auto command = GraphicsPresent::GetCommandBuffer();
				m_Selector.Present(command,cameras);
				Reject([&]{m_Selector.Present(command,cameras);});
			}
		private:
			template<typename Callback> void Failure(const char* label,Callback&& callback,bool preflight=true,const char* errorSubstring="")
			{
				const auto original = m_Selector.GetActive();
				const auto settings = original->GetAsset().SerializePayload();
				const auto factories = m_Probe->Factories;
				bool rejected = false;
				try { callback(); }
				catch (const std::exception& error)
				{
					Check(std::string(error.what()).find(errorSubstring) != std::string::npos,"unexpected candidate failure reason.");
					rejected = true;
				}
				Check(rejected && original == m_Selector.GetActive() && settings == m_Selector.GetActive()->GetAsset().SerializePayload(),std::string("rollback failed: ")+label);
				if (preflight) Check(factories == m_Probe->Factories,"preflight failure invoked pass factories.");
				GE_INFO("Pipeline asset rejection passed: {}.",label);
			}
			void Failures()
			{
				Failure("missing asset",[&]{m_Selector.Activate(std::filesystem::path("Missing.gasset"),m_Registry);});
				auto invalid = m_Warm; invalid.Version = 99;
				Failure("unknown version",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Type = "Unknown.Pipeline";
				Failure("unknown pipeline type",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Passes[0].Type = "Unknown.Pass";
				Failure("unknown pass type",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Resources.at("ColorShader").Path = "MissingShader.gasset";
				Failure("missing dependency",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Resources.at("ColorShader").Type = "texture";
				Failure("wrong dependency type",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Resources.at("ColorShader").Path = "../Escape.gasset";
				Failure("dependency path escape",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Targets[0].Width = Graphics::GetRenderDevice().QueryRenderTargetFormat(FRAME_BUFFER_TEXTURE_FORMAT_RGBA8).MaxWidth + 1;
				Failure("backend target limit",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; invalid.Passes[0].Settings["red"] = "nan";
				Failure("invalid pass settings",[&]{m_Selector.Activate(invalid,m_Registry);});
				invalid = m_Warm; std::swap(invalid.Passes[0],invalid.Passes[1]);
				Failure("invalid pass ordering",[&]{m_Selector.Activate(invalid,m_Registry);});
				Failure("malformed payload",[&]{RenderPipelineAsset::ParsePayload("version: [");});
				Failure("unknown payload field",[&]{RenderPipelineAsset::ParsePayload(m_Warm.SerializePayload()+"\nunknown: value");});
				Failure("duplicate payload field",[&]{RenderPipelineAsset::ParsePayload(m_Warm.SerializePayload()+"\nversion: 1");});
				for (const std::string phase : {"configure","prepare","cycle"})
				{
					invalid = m_Warm; invalid.Passes.push_back({"Fault","Example.Fault.v1",{{"phase",phase}}});
					Failure(phase.c_str(),[&]{m_Selector.Activate(invalid,m_Registry);},false);
				}
				invalid = m_Warm; invalid.Passes.push_back({"Null","Example.Null.v1",{}});
				Failure("null factory",[&]{m_Selector.Activate(invalid,m_Registry);},false);
				invalid = m_Warm; invalid.Resources.at("ColorShader").Path = "Assets/Shaders/PipelineAssetInvalid.gasset";
				Failure("shader compilation",[&]{m_Selector.Activate(invalid,m_Registry);},false,"Failed to compile shader stage");
				const char* parserCheck = std::getenv("GENGINE_RENDER_PIPELINE_PARSER_CHECK");
				if (parserCheck && std::string(parserCheck) == "1")
				{
					for (const std::string file : { "PipelineParserInvalidState", "PipelineParserInvalidStructure" })
					{
						const auto cache = std::filesystem::path(Application::Get().GetConfig()->GetShaderCacheDirectory()) / file;
						Check(!std::filesystem::exists(cache), "malformed shader fixture has stale cache.");
						invalid = m_Warm;
						invalid.Resources.at("ColorShader").Path = "Assets/Shaders/" + file + ".gasset";
						for (int attempt = 0; attempt < 2; ++attempt)
						{
							Failure(file.c_str(), [&]{m_Selector.Activate(invalid,m_Registry);}, false, "Shader parse error ");
							Check(!std::filesystem::exists(cache), "malformed shader published a cache.");
						}
					}
					GE_INFO("Shader parser rollback passed: 4 cases.");
				}
			}
			RenderPipelineAssetRegistry m_Registry;
			RenderPipelineSelector m_Selector;
			Ref<ProbeState> m_Probe;
			RenderPipelineAsset m_Warm,m_Cool;
		};
	}
	Ref<PipelineAssetChecks> CreatePipelineAssetChecks() { return CreateRef<Checks>(); }
}

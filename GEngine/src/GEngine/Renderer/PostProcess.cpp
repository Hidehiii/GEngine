#include "GEpch.h"
#include "GEngine/Renderer/PostProcess.h"
#include "GEngine/Renderer/RenderPipelineAsset.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/Sampler.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace GEngine
{
	namespace
	{
		void RequirePost(bool valid, const std::string& message)
		{
			if (!valid) throw std::invalid_argument("Post-process: " + message);
		}
		void ValidateDevice()
		{
			const auto capabilities = Graphics::GetCapabilities();
			RequirePost(capabilities.Sampler && capabilities.UniformBuffer, "sampler and uniform buffers are required.");
			for (auto format : {FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F, FRAME_BUFFER_TEXTURE_FORMAT_RGBA8})
			{
				const auto support = Graphics::GetRenderDevice().QueryRenderTargetFormat(format);
				RequirePost(support.ColorAttachment && support.Sampled && support.SupportsSamples(1), "required HDR/output format is unsupported.");
			}
		}
	}
	void PostProcessSettings::Validate() const
	{
		RequirePost(Encoding == PostProcessEncoding::Linear || Encoding == PostProcessEncoding::SRGB, "unknown output encoding.");
		RequirePost(Effects.size() <= 4, "at most four effect slots are supported.");
		std::set<std::string> ids;
		for (const auto& effect : Effects)
		{
			RequirePost(!effect.ID.empty() && ids.insert(effect.ID).second, "duplicate or empty effect ID.");
			RequirePost(effect.Type == PostProcessEffectType::Exposure || effect.Type == PostProcessEffectType::Reinhard, "unknown effect type.");
			RequirePost(std::isfinite(effect.EV) && effect.EV >= -16 && effect.EV <= 16, "EV must be finite and in [-16,16].");
			RequirePost(effect.Type != PostProcessEffectType::Reinhard || effect.EV == 0, "Reinhard has no EV parameter.");
		}
	}
	void PostProcessStackSpecification::Validate(const RenderPipelineAsset& asset) const
	{
		RequirePost(!Name.empty() && Input != Output, "name and distinct input/output are required.");
		Settings.Validate();
		const auto& input = asset.GetTarget(Input);
		const auto& output = asset.GetTarget(Output);
		RequirePost(input.ColorFormats == std::vector<FrameBufferTextureFormat>{FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F} && input.Samples == 1,
			"input must be a resolved single RGBA16F attachment.");
		RequirePost(output.ColorFormats == std::vector<FrameBufferTextureFormat>{FRAME_BUFFER_TEXTURE_FORMAT_RGBA8} && output.Samples == 1 &&
			output.DepthStencilFormat == FRAME_BUFFER_TEXTURE_FORMAT_NONE && input.Width == output.Width && input.Height == output.Height,
			"output must be same-size single-sample RGBA8 without depth.");
		RequirePost(asset.GetResource(Shader).Type == "shader", "shader dependency is required.");
	}
	PostProcessStackPass::PostProcessStackPass(std::string name, PostProcessStackSpecification specification,
		Ref<const RenderPipelineAssetContext> context, PostProcessSettingsProvider settings)
		: m_Name(std::move(name)), m_Specification(std::move(specification)), m_Context(std::move(context)), m_Settings(std::move(settings))
	{
		RequirePost(bool(m_Context), "asset context is required.");
		m_Specification.Validate(m_Context->GetAsset());
		ValidateDevice();
	}
	void PostProcessStackPass::Configure(RenderPipelineBuilder& builder)
	{
		m_Sources.clear();
		m_Intermediate.clear();
		m_Materials.clear();
		m_Pipelines.clear();
		auto& graph = builder.GetGraph();
		const auto& input = m_Context->GetAsset().GetTarget(m_Specification.Input);
		const size_t count = std::max(size_t(1), m_Specification.Settings.Effects.size());
		m_Pipelines.resize(count);
		auto source = m_Context->GetTarget(m_Specification.Input);
		for (size_t i = 0; i < count; ++i)
		{
			m_Sources.push_back(source);
			auto output = m_Context->GetTarget(m_Specification.Output);
			if (i + 1 < count)
			{
				RenderGraph::AttachmentSpecification attachment;
				attachment.ColorFormats = {FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F};
				output = builder.CreateRenderTarget(m_Name + ".HDR" + std::to_string(i), attachment, input.Width, input.Height);
				m_Intermediate.push_back(output);
			}
			const auto pass = graph.BuildGraphicsPass(m_Name + ".Stage" + std::to_string(i), output,
				[this,i](const Ref<CommandBuffer>& command){command->Render(m_Pipelines.at(i),0);}).GetHandle();
			graph.Read(pass, graph.GetColorAttachment(source), RenderGraph::ResourceState::ShaderRead, GraphicsPipelineStage::FragmentShader);
			source = output;
		}
	}
	void PostProcessStackPass::Prepare(const RenderGraph& graph)
	{
		auto& device = Graphics::GetRenderDevice();
		const auto shader = Shader::Create(m_Context->GetAsset().GetResource(m_Specification.Shader).Path.u8string());
		RequirePost(shader->GetPassReflections().size() == 1 && shader->GetPropertyType("PostParameters") == SHADER_PROPERTY_TYPE_FLOAT_4 &&
			shader->GetPropertyType("PostInput") == SHADER_PROPERTY_TYPE_TEXTURE_2D &&
			shader->GetPropertyType("PostSampler") == SHADER_PROPERTY_TYPE_SAMPLER, "shader does not implement the post-process v1 contract.");
		const float top = Graphics::GetCapabilities().AttachmentUvStartsAtTop ? 0.0f : 1.0f;
		const float vertices[][5] = {{-1,-1,0,0,1-top},{1,-1,0,1,1-top},{1,1,0,1,top},{-1,1,0,0,top}};
		const uint32_t indices[] = {0,1,2,2,3,0};
		auto buffer = device.CreateVertexBuffer(vertices,sizeof(vertices),0,VERTEX_TOPOLOGY_TRIANGLE);
		buffer->SetIndexBuffer(device.CreateIndexBuffer(indices,6));
		SamplerSpecification sampling;
		sampling.MinFilter = sampling.MagFilter = sampling.MipmapFilter = SAMPLER_FILTER_NEAREST;
		sampling.WrapU = sampling.WrapV = sampling.WrapW = SAMPLER_ADDRESS_CLAMP_TO_EDGE;
		const auto sampler = device.CreateSampler(sampling);
		for (size_t i = 0; i < m_Pipelines.size(); ++i)
		{
			auto material = device.CreateMaterial(shader,m_Name + ".Stage" + std::to_string(i));
			material->SetBlendMode(BLEND_MODE_NONE,BLEND_FACTOR_ONE,BLEND_FACTOR_ZERO,0);
			material->SetEnableDepthWrite(false,0);
			material->SetDepthTestOp(COMPARE_OP_ALWAYS,0);
			material->SetCullMode(CULL_MODE_NONE,0);
			material->SetResource<Texture2D>("PostInput",graph.GetTexture2D(graph.GetColorAttachment(m_Sources[i])));
			material->SetResource<Sampler>("PostSampler",sampler);
			m_Materials.push_back(material);
			m_Pipelines[i] = device.CreateGraphicsPipeline(material,buffer);
		}
		Apply(m_Specification.Settings);
	}
	void PostProcessStackPass::Apply(const PostProcessSettings& settings)
	{
		settings.Validate();
		RequirePost(settings.Effects.size() == m_Specification.Settings.Effects.size(), "stage count changes require pipeline replacement.");
		// Keep stable identities/types while allowing their ordered settings to change.
		for (const auto& effect : settings.Effects)
		{
			const auto& original = m_Specification.Settings.Effects;
			RequirePost(std::any_of(original.begin(),original.end(),[&](const auto& item){return item.ID == effect.ID && item.Type == effect.Type;}), "live settings change effect identity/type.");
		}
		for (size_t i = 0; i < m_Materials.size(); ++i)
		{
			float operation = 0, multiplier = 1;
			if (settings.Enabled && !settings.Effects.empty() && settings.Effects[i].Enabled)
			{
				const auto& effect = settings.Effects[i];
				operation = effect.Type == PostProcessEffectType::Exposure ? 1.0f : 2.0f;
				multiplier = std::exp2(effect.EV);
			}
			const float encode = i + 1 == m_Materials.size() && settings.Encoding == PostProcessEncoding::SRGB ? 1.0f : 0.0f;
			m_Materials[i]->SetConstant<Vector4>("PostParameters",Vector4{operation,multiplier,encode,0});
		}
	}
	void PostProcessStackPass::Execute(RenderPipelineContext& context)
	{
		if (m_Settings) Apply(m_Settings(context));
	}
	void RegisterPostProcessPass(RenderPipelineAssetRegistry& registry, PostProcessSettingsProvider settings)
	{
		registry.Register("GEngine.PostProcess.v1", [](const auto& pass,const auto& asset,const auto&)
		{
			RequirePost(pass.Settings.size() == 1 && pass.Settings.count("stack"), "pass needs exactly one stack setting.");
			asset.GetPostProcessStack(pass.Settings.at("stack")).Validate(asset);
			ValidateDevice();
		}, [settings](const auto& pass,const auto& context)->Ref<RenderPipelinePass>
		{
			return CreateRef<PostProcessStackPass>(pass.Name,context->GetAsset().GetPostProcessStack(pass.Settings.at("stack")),context,settings);
		});
	}
}

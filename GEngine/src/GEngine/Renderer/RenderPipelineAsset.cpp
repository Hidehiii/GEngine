#include "GEpch.h"
#include "GEngine/Renderer/RenderPipelineAsset.h"
#include "GEngine/Asset/GAsset.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Runtime/ResourcePack.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>

namespace GEngine
{
	namespace
	{
		void Require(bool condition, const std::string& message)
		{
			if (!condition) throw std::invalid_argument("Render pipeline asset: " + message);
		}
		bool Identifier(const std::string& value)
		{
			if (value.empty()) return false;
			for (unsigned char c : value)
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
					(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
			return true;
		}
		void CheckMap(const YAML::Node& node, std::initializer_list<const char*> fields)
		{
			Require(node.IsMap(), "expected a map.");
			std::set<std::string> keys;
			for (const auto& item : node)
			{
				Require(item.first.IsScalar(), "map keys must be strings.");
				const auto key = item.first.as<std::string>();
				Require(keys.insert(key).second, "duplicate field: " + key);
				bool known = false;
				for (auto field : fields) if (key == field) known = true;
				Require(known, "unknown field: " + key);
			}
		}
		std::string String(const YAML::Node& node)
		{
			Require(node.IsScalar(), "expected a scalar string.");
			return node.as<std::string>();
		}
		uint32_t Unsigned(const YAML::Node& node)
		{
			const auto value = String(node);
			Require(!value.empty() && value.find_first_not_of("0123456789") == std::string::npos, "expected an unsigned integer.");
			const auto number = std::stoull(value);
			Require(number <= std::numeric_limits<uint32_t>::max(), "integer exceeds uint32.");
			return static_cast<uint32_t>(number);
		}
		RenderPipelineSettings ReadSettings(const YAML::Node& node)
		{
			RenderPipelineSettings settings;
			if (!node) return settings;
			Require(node.IsMap(), "settings must be a map of scalar strings.");
			for (const auto& item : node)
			{
				const auto key = String(item.first);
				Require(Identifier(key), "invalid setting name: " + key);
				Require(settings.emplace(key, String(item.second)).second, "duplicate setting: " + key);
			}
			return settings;
		}
		void WriteSettings(YAML::Emitter& out, const RenderPipelineSettings& settings)
		{
			out << YAML::BeginMap;
			for (const auto& item : settings) out << YAML::Key << item.first << YAML::Value << item.second;
			out << YAML::EndMap;
		}
		const std::vector<std::pair<FrameBufferTextureFormat, const char*>> Formats = {
			{ FRAME_BUFFER_TEXTURE_FORMAT_NONE, "None" }, { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8, "RGBA8" },
			{ FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F, "RGBA16F" },
			{ FRAME_BUFFER_TEXTURE_FORMAT_R32F, "R32F" }, { FRAME_BUFFER_TEXTURE_FORMAT_RG16F, "RG16F" },
			{ FRAME_BUFFER_TEXTURE_FORMAT_R32I, "R32I" }, { FRAME_BUFFER_TEXTURE_FORMAT_RG16I, "RG16I" },
			{ FRAME_BUFFER_TEXTURE_FORMAT_R32UI, "R32UI" }, { FRAME_BUFFER_TEXTURE_FORMAT_RG16UI, "RG16UI" },
			{ FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8, "D24S8" }, { FRAME_BUFFER_TEXTURE_FORMAT_DEPTH, "D32" }
		};
		FrameBufferTextureFormat ReadFormat(const YAML::Node& node)
		{
			const auto name = String(node);
			for (const auto& format : Formats) if (name == format.second) return format.first;
			throw std::invalid_argument("Unknown pipeline attachment format: " + name);
		}
		const char* FormatName(FrameBufferTextureFormat value)
		{
			for (const auto& format : Formats) if (value == format.first) return format.second;
			throw std::invalid_argument("Unknown pipeline attachment format value.");
		}
	}

	RenderGraph::AttachmentSpecification RenderPipelineTargetAsset::GetSpecification() const
	{
		RenderGraph::AttachmentSpecification spec;
		spec.ColorFormats = ColorFormats;
		spec.DepthStencilFormat = DepthStencilFormat;
		spec.Samples = Samples;
		return spec;
	}
	const RenderPipelineTargetAsset& RenderPipelineAsset::GetTarget(const std::string& name) const
	{
		for (const auto& target : Targets) if (target.Name == name) return target;
		throw std::invalid_argument("Unknown pipeline target: " + name);
	}
	const RenderPipelineResourceAsset& RenderPipelineAsset::GetResource(const std::string& name) const
	{
		const auto found = Resources.find(name);
		if (found == Resources.end()) throw std::invalid_argument("Unknown pipeline resource: " + name);
		return found->second;
	}
	const CameraStackSpecification& RenderPipelineAsset::GetCameraStack(const std::string& name) const
	{
		for (const auto& stack : CameraStacks) if (stack.Name == name) return stack;
		throw std::invalid_argument("Unknown camera stack: " + name);
	}
	const PostProcessStackSpecification& RenderPipelineAsset::GetPostProcessStack(const std::string& name) const
	{
		for (const auto& stack : PostProcessStacks) if (stack.Name == name) return stack;
		throw std::invalid_argument("Unknown post-process stack: " + name);
	}
	void RenderPipelineAsset::ValidateStructure() const
	{
		Require(Version >= 1 && Version <= 3, "unsupported payload version.");
		Require(Version >= 2 || CameraStacks.empty(), "camera stacks require payload version >= 2.");
		Require(Version == 3 || PostProcessStacks.empty(), "post-process stacks require payload version 3.");
		Require(Type == "GEngine.Composable", "unknown pipeline type: " + Type);
		Require(!Targets.empty() && !Passes.empty(), "targets and passes must not be empty.");
		for (const auto& setting : Settings) Require(Identifier(setting.first), "invalid pipeline setting name.");
		std::set<std::string> names;
		for (const auto& target : Targets)
		{
			Require(Identifier(target.Name) && names.insert(target.Name).second, "invalid or duplicate target name.");
			Require(target.Width && target.Height, "target dimensions must be nonzero.");
			Require(target.Samples && target.Samples <= 64 && !(target.Samples & (target.Samples - 1)), "invalid sample count.");
			Require(!target.ColorFormats.empty(), "target requires a color attachment.");
			for (auto color : target.ColorFormats)
				Require(color == FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F || (color >= FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 && color < FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8), "invalid color attachment.");
			Require(target.DepthStencilFormat == FRAME_BUFFER_TEXTURE_FORMAT_NONE ||
				target.DepthStencilFormat == FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8 ||
				target.DepthStencilFormat == FRAME_BUFFER_TEXTURE_FORMAT_DEPTH, "invalid depth attachment.");
		}
		names.clear();
		for (const auto& stack : CameraStacks)
		{
			Require(Identifier(stack.Name) && names.insert(stack.Name).second, "invalid or duplicate camera stack name.");
			const auto& target = GetTarget(stack.Target);
			for (const auto& node : stack.Nodes) Require(Identifier(node.Camera), "invalid camera ID.");
			stack.Validate(target.Width,target.Height,target.Samples,target.DepthStencilFormat != FRAME_BUFFER_TEXTURE_FORMAT_NONE);
		}
		names.clear();
		for (const auto& stack : PostProcessStacks)
		{
			Require(Identifier(stack.Name) && names.insert(stack.Name).second, "invalid or duplicate post-process stack name.");
			for (const auto& effect : stack.Settings.Effects) Require(Identifier(effect.ID), "invalid effect ID.");
			stack.Validate(*this);
		}
		names.clear();
		for (const auto& pass : Passes)
		{
			Require(Identifier(pass.Name) && names.insert(pass.Name).second, "invalid or duplicate pass name.");
			Require(Identifier(pass.Type), "invalid pass type ID.");
			for (const auto& setting : pass.Settings) Require(Identifier(setting.first), "invalid pass setting name.");
		}
		for (const auto& item : Resources)
		{
			Require(Identifier(item.first) && Identifier(item.second.Type), "invalid resource name/type.");
			const auto& path = item.second.Path;
			Require(!path.empty() && !path.is_absolute() && !path.has_root_name() && path.extension() == ".gasset", "resource path must be a relative GAsset path.");
			for (const auto& part : path) Require(part != "..", "resource paths may not escape the runtime root.");
		}
	}

	RenderPipelineAsset RenderPipelineAsset::ParsePayload(const std::string& payload)
	{
		const auto root = YAML::Load(payload);
		CheckMap(root, { "version", "pipelineType", "settings", "targets", "resources", "passes", "cameraStacks", "postProcessStacks" });
		RenderPipelineAsset asset;
		asset.Version = Unsigned(root["version"]);
		asset.Type = String(root["pipelineType"]);
		asset.Settings = ReadSettings(root["settings"]);
		const auto targets = root["targets"];
		Require(targets.IsSequence(), "targets must be an ordered sequence.");
		for (const auto& node : targets)
		{
			CheckMap(node, { "name", "width", "height", "samples", "colors", "depth" });
			RenderPipelineTargetAsset target;
			target.Name = String(node["name"]);
			target.Width = Unsigned(node["width"]);
			target.Height = Unsigned(node["height"]);
			if (node["samples"]) target.Samples = Unsigned(node["samples"]);
			Require(node["colors"].IsSequence(), "colors must be a sequence.");
			for (const auto& color : node["colors"]) target.ColorFormats.push_back(ReadFormat(color));
			if (node["depth"]) target.DepthStencilFormat = ReadFormat(node["depth"]);
			asset.Targets.push_back(std::move(target));
		}
		if (const auto resources = root["resources"])
		{
			Require(resources.IsMap(), "resources must be a map.");
			for (const auto& item : resources)
			{
				CheckMap(item.second, { "path", "type" });
				Require(asset.Resources.emplace(String(item.first), RenderPipelineResourceAsset{
					String(item.second["path"]), String(item.second["type"]) }).second, "duplicate resource name.");
			}
		}
		Require(root["passes"].IsSequence(), "passes must be an ordered sequence.");
		for (const auto& node : root["passes"])
		{
			CheckMap(node, { "name", "type", "settings" });
			asset.Passes.push_back({ String(node["name"]), String(node["type"]), ReadSettings(node["settings"]) });
		}
		if (const auto stacks = root["cameraStacks"])
		{
			Require(asset.Version >= 2 && stacks.IsSequence(), "cameraStacks requires version >= 2 and a sequence.");
			for (const auto& stackNode : stacks)
			{
				CheckMap(stackNode,{"name","target","base","nodes"});
				CameraStackSpecification stack;
				stack.Name = String(stackNode["name"]); stack.Target = String(stackNode["target"]); stack.BaseCamera = String(stackNode["base"]);
				Require(stackNode["nodes"].IsSequence(), "camera nodes must be a sequence.");
				for (const auto& input : stackNode["nodes"])
				{
					CheckMap(input,{"camera","role","enabled","target","samples","viewport","color","depth","overlays"});
					CameraStackNode node;
					node.Camera = String(input["camera"]);
					const auto role = String(input["role"]);
					Require(role == "base" || role == "overlay", "unknown camera role.");
					node.Role = role == "base" ? CameraStackRole::Base : CameraStackRole::Overlay;
					if (input["enabled"]) node.Enabled = input["enabled"].as<bool>();
					if (input["target"]) node.Target = String(input["target"]);
					if (input["samples"]) node.Samples = Unsigned(input["samples"]);
					if (const auto viewport = input["viewport"])
					{
						Require(viewport.IsSequence() && viewport.size() == 4, "viewport requires x/y/width/height.");
						node.Viewport = {viewport[0].as<float>(),viewport[1].as<float>(),viewport[2].as<float>(),viewport[3].as<float>()};
					}
					auto operation = [&](const YAML::Node& value)
					{
						const auto op = String(value);
						Require(op == "clear" || op == "load" || op == "none", "unknown camera load operation.");
						return op == "clear" ? RENDER_PASS_BEGINE_OP_CLEAR : op == "load" ? RENDER_PASS_BEGINE_OP_LOAD : RENDER_PASS_BEGINE_OP_NONE;
					};
					if (input["color"]) node.Color = operation(input["color"]);
					if (input["depth"]) node.Depth = operation(input["depth"]);
					if (const auto overlays = input["overlays"])
					{
						Require(overlays.IsSequence(), "overlay edges must be a sequence.");
						for (const auto& overlay : overlays) node.Overlays.push_back(String(overlay));
					}
					stack.Nodes.push_back(std::move(node));
				}
				asset.CameraStacks.push_back(std::move(stack));
			}
		}
		if (const auto stacks = root["postProcessStacks"])
		{
			Require(asset.Version == 3 && stacks.IsSequence(), "postProcessStacks requires version 3 and a sequence.");
			for (const auto& input : stacks)
			{
				CheckMap(input,{"name","input","output","shader","enabled","encoding","effects"});
				PostProcessStackSpecification stack;
				stack.Name = String(input["name"]); stack.Input = String(input["input"]);
				stack.Output = String(input["output"]); stack.Shader = String(input["shader"]);
				if (input["enabled"]) stack.Settings.Enabled = input["enabled"].as<bool>();
				const auto encoding = String(input["encoding"]);
				Require(encoding == "linear" || encoding == "srgb", "unknown post-process encoding.");
				stack.Settings.Encoding = encoding == "srgb" ? PostProcessEncoding::SRGB : PostProcessEncoding::Linear;
				Require(input["effects"].IsSequence(), "effects must be an ordered sequence.");
				for (const auto& item : input["effects"])
				{
					CheckMap(item,{"id","type","enabled","ev"});
					PostProcessEffect effect;
					effect.ID = String(item["id"]);
					const auto type = String(item["type"]);
					Require(type == "exposure" || type == "reinhard", "unknown post-process effect.");
					effect.Type = type == "exposure" ? PostProcessEffectType::Exposure : PostProcessEffectType::Reinhard;
					if (item["enabled"]) effect.Enabled = item["enabled"].as<bool>();
					if (item["ev"]) effect.EV = item["ev"].as<float>();
					stack.Settings.Effects.push_back(effect);
				}
				asset.PostProcessStacks.push_back(std::move(stack));
			}
		}
		asset.ValidateStructure();
		return asset;
	}
	std::string RenderPipelineAsset::SerializePayload() const
	{
		ValidateStructure();
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "version" << YAML::Value << Version
			<< YAML::Key << "pipelineType" << YAML::Value << Type << YAML::Key << "settings" << YAML::Value;
		WriteSettings(out, Settings);
		out << YAML::Key << "targets" << YAML::Value << YAML::BeginSeq;
		for (const auto& target : Targets)
		{
			out << YAML::BeginMap << YAML::Key << "name" << YAML::Value << target.Name
				<< YAML::Key << "width" << YAML::Value << target.Width << YAML::Key << "height" << YAML::Value << target.Height
				<< YAML::Key << "samples" << YAML::Value << target.Samples << YAML::Key << "colors" << YAML::Value << YAML::BeginSeq;
			for (auto color : target.ColorFormats) out << FormatName(color);
			out << YAML::EndSeq << YAML::Key << "depth" << YAML::Value << FormatName(target.DepthStencilFormat) << YAML::EndMap;
		}
		out << YAML::EndSeq << YAML::Key << "resources" << YAML::Value << YAML::BeginMap;
		for (const auto& item : Resources)
			out << YAML::Key << item.first << YAML::Value << YAML::BeginMap << YAML::Key << "path" << YAML::Value
				<< item.second.Path.generic_u8string() << YAML::Key << "type" << YAML::Value << item.second.Type << YAML::EndMap;
		out << YAML::EndMap << YAML::Key << "passes" << YAML::Value << YAML::BeginSeq;
		for (const auto& pass : Passes)
		{
			out << YAML::BeginMap << YAML::Key << "name" << YAML::Value << pass.Name << YAML::Key << "type" << YAML::Value << pass.Type
				<< YAML::Key << "settings" << YAML::Value;
			WriteSettings(out, pass.Settings);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		if (Version >= 2)
		{
			out << YAML::Key << "cameraStacks" << YAML::Value << YAML::BeginSeq;
			for (const auto& stack : CameraStacks)
			{
				out << YAML::BeginMap << YAML::Key << "name" << YAML::Value << stack.Name << YAML::Key << "target" << YAML::Value << stack.Target
					<< YAML::Key << "base" << YAML::Value << stack.BaseCamera << YAML::Key << "nodes" << YAML::Value << YAML::BeginSeq;
				for (const auto& node : stack.Nodes)
				{
					auto op = [](RenderPassBeginOperation value){return value == RENDER_PASS_BEGINE_OP_CLEAR ? "clear" : value == RENDER_PASS_BEGINE_OP_LOAD ? "load" : "none";};
					out << YAML::BeginMap << YAML::Key << "camera" << YAML::Value << node.Camera << YAML::Key << "role" << YAML::Value
						<< (node.Role == CameraStackRole::Base ? "base" : "overlay") << YAML::Key << "enabled" << YAML::Value << node.Enabled
						<< YAML::Key << "target" << YAML::Value << node.Target << YAML::Key << "samples" << YAML::Value << node.Samples
						<< YAML::Key << "viewport" << YAML::Value << YAML::Flow << YAML::BeginSeq << node.Viewport.X << node.Viewport.Y << node.Viewport.Width << node.Viewport.Height << YAML::EndSeq
						<< YAML::Key << "color" << YAML::Value << op(node.Color) << YAML::Key << "depth" << YAML::Value << op(node.Depth)
						<< YAML::Key << "overlays" << YAML::Value << YAML::BeginSeq;
					for (const auto& overlay : node.Overlays) out << overlay;
					out << YAML::EndSeq << YAML::EndMap;
				}
				out << YAML::EndSeq << YAML::EndMap;
			}
			out << YAML::EndSeq;
		}
		if (Version == 3)
		{
			out << YAML::Key << "postProcessStacks" << YAML::Value << YAML::BeginSeq;
			for (const auto& stack : PostProcessStacks)
			{
				out << YAML::BeginMap << YAML::Key << "name" << YAML::Value << stack.Name
					<< YAML::Key << "input" << YAML::Value << stack.Input << YAML::Key << "output" << YAML::Value << stack.Output
					<< YAML::Key << "shader" << YAML::Value << stack.Shader << YAML::Key << "enabled" << YAML::Value << stack.Settings.Enabled
					<< YAML::Key << "encoding" << YAML::Value << (stack.Settings.Encoding == PostProcessEncoding::SRGB ? "srgb" : "linear")
					<< YAML::Key << "effects" << YAML::Value << YAML::BeginSeq;
				for (const auto& effect : stack.Settings.Effects)
					out << YAML::BeginMap << YAML::Key << "id" << YAML::Value << effect.ID
						<< YAML::Key << "type" << YAML::Value << (effect.Type == PostProcessEffectType::Exposure ? "exposure" : "reinhard")
						<< YAML::Key << "enabled" << YAML::Value << effect.Enabled << YAML::Key << "ev" << YAML::Value << effect.EV << YAML::EndMap;
				out << YAML::EndSeq << YAML::EndMap;
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;
		if (!out.good()) throw std::runtime_error("Cannot serialize render pipeline asset.");
		return out.c_str();
	}
	RenderPipelineAsset RenderPipelineAsset::Load(const std::filesystem::path& path)
	{
		const auto envelope = LoadGAsset(path, "render-pipeline");
		Require(envelope.Format == "render-pipeline", "unsupported GAsset payload format.");
		return ParsePayload(std::string(envelope.Payload.begin(), envelope.Payload.end()));
	}
	void RenderPipelineAsset::Save(const std::filesystem::path& path) const
	{
		Require(path.extension() == ".gasset", "output path must use .gasset.");
		const auto payload = SerializePayload();
		EnsureResourceWritable(path);
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "schemaVersion" << YAML::Value << 1
			<< YAML::Key << "type" << YAML::Value << "render-pipeline" << YAML::Key << "format" << YAML::Value << "render-pipeline"
			<< YAML::Key << "payloadEncoding" << YAML::Value << "text" << YAML::Key << "payload" << YAML::Value << YAML::Literal << payload << YAML::EndMap;
		if (!out.good()) throw std::runtime_error("Cannot serialize pipeline GAsset envelope.");
		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		if (!file || !(file << out.c_str() << '\n')) throw std::runtime_error("Cannot write pipeline GAsset: " + path.u8string());
		file.close();
		if (!file) throw std::runtime_error("Cannot close pipeline GAsset: " + path.u8string());
	}

	RenderGraph::TargetHandle RenderPipelineAssetContext::GetTarget(const std::string& name) const
	{
		const auto found = m_Targets.find(name);
		if (found == m_Targets.end()) throw std::invalid_argument("Unknown configured pipeline target: " + name);
		return found->second;
	}
	RenderPipelineAssetInstance::RenderPipelineAssetInstance(RenderPipelineAsset asset, std::vector<RenderPipelinePassFactory> factories)
		: m_Asset(std::move(asset)), m_Factories(std::move(factories)) {}
	void RenderPipelineAssetInstance::Configure(RenderPipelineBuilder& builder)
	{
		auto context = Ref<RenderPipelineAssetContext>(new RenderPipelineAssetContext(m_Asset));
		for (const auto& target : m_Asset.Targets)
			context->m_Targets.emplace(target.Name, builder.CreateRenderTarget(target.Name, target.GetSpecification(), target.Width, target.Height));
		m_Context = context;
		for (size_t i = 0; i < m_Asset.Passes.size(); ++i)
		{
			auto pass = m_Factories[i](m_Asset.Passes[i], m_Context);
			Require(pass && pass->GetName() == m_Asset.Passes[i].Name, "factory returned a null or incorrectly named pass.");
			AddPass(pass);
		}
	}
	RenderGraph::TargetHandle RenderPipelineAssetInstance::GetTarget(const std::string& name) const
	{
		if (!m_Context) throw std::logic_error("Pipeline asset instance is not configured.");
		return m_Context->GetTarget(name);
	}
	void RenderPipelineAssetRegistry::Register(const std::string& type, RenderPipelinePassValidator validate, RenderPipelinePassFactory create)
	{
		Require(Identifier(type) && validate && create, "pass registration requires a type, validator and factory.");
		Require(m_Entries.emplace(type, Entry{ std::move(validate), std::move(create) }).second, "duplicate pass type: " + type);
	}
	Ref<RenderPipelineAssetInstance> RenderPipelineAssetRegistry::Create(RenderPipelineAsset asset) const
	{
		const auto entries = m_Entries;
		asset.ValidateStructure();
		const auto capabilities = Graphics::GetCapabilities();
		Require(capabilities.RenderPass && capabilities.FrameBuffer && capabilities.Texture2D, "backend lacks render target support.");
		for (const auto& target : asset.Targets)
		{
			Require(target.ColorFormats.size() <= capabilities.MaxColorAttachments, "color attachment count exceeds the device limit.");
			RenderPassSpecification spec{};
			spec.RenderTargets = target.ColorFormats;
			spec.DepthStencil = target.DepthStencilFormat;
			spec.Samples = target.Samples;
			Graphics::GetRenderDevice().ValidateRenderTarget(spec, target.Width, target.Height);
		}
		for (const auto& resource : asset.Resources) LoadGAsset(resource.second.Path, resource.second.Type);
		std::vector<RenderPipelinePassFactory> factories;
		for (const auto& pass : asset.Passes)
		{
			const auto found = entries.find(pass.Type);
			Require(found != entries.end(), "unregistered pass type: " + pass.Type);
			found->second.Validate(pass, asset, capabilities);
			factories.push_back(found->second.Create);
		}
		return Ref<RenderPipelineAssetInstance>(new RenderPipelineAssetInstance(asset, std::move(factories)));
	}
	void RenderPipelineSelector::ActivateCandidate(const RenderPipelineAsset& asset, const RenderPipelineAssetRegistry& registry)
	{
		auto candidate = registry.Create(asset);
		candidate->Prepare();
		m_Active.swap(candidate);
	}
	void RenderPipelineSelector::Activate(const RenderPipelineAsset& asset, const RenderPipelineAssetRegistry& registry)
	{
		if (m_State != State::Idle) throw std::logic_error("Pipeline switching requires a complete frame boundary.");
		m_State = State::Activating;
		try { ActivateCandidate(asset, registry); m_State = State::Idle; }
		catch (...) { m_State = State::Idle; throw; }
	}
	void RenderPipelineSelector::Activate(const std::filesystem::path& path, const RenderPipelineAssetRegistry& registry)
	{
		if (m_State != State::Idle) throw std::logic_error("Pipeline switching requires a complete frame boundary.");
		m_State = State::Activating;
		try { ActivateCandidate(RenderPipelineAsset::Load(path), registry); m_State = State::Idle; }
		catch (...) { m_State = State::Idle; throw; }
	}
	void RenderPipelineSelector::Execute(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras)
	{
		if (m_State != State::Idle || !m_Active || !command) throw std::logic_error("Pipeline selection Execute requires an active idle pipeline and command.");
		m_State = State::Executing;
		try { m_Active->Execute(command, cameras); m_State = State::AwaitingPresent; }
		catch (...) { m_State = State::Failed; throw; }
	}
	void RenderPipelineSelector::Present(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras)
	{
		if (m_State != State::AwaitingPresent || !command) throw std::logic_error("Pipeline selection Present requires Execute and a command.");
		m_State = State::Presenting;
		try { m_Active->Present(command, cameras); m_State = State::Idle; }
		catch (...) { m_State = State::Failed; throw; }
	}
	void RenderPipelineSelector::Clear()
	{
		if (m_State != State::Idle && m_State != State::Failed) throw std::logic_error("Pipeline selection Clear requires an idle or failed selection.");
		m_Active.reset();
		m_State = State::Idle;
	}
}

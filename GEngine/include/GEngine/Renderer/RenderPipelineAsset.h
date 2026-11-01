#pragma once

#include "GEngine/Renderer/RenderPipeline.h"
#include "GEngine/Renderer/CameraStack.h"
#include "GEngine/Renderer/PostProcess.h"
#include <filesystem>
#include <functional>
#include <map>

namespace GEngine
{
	struct GraphicsCapabilities;
	using RenderPipelineSettings = std::map<std::string, std::string>;

	struct GENGINE_API RenderPipelineTargetAsset
	{
		std::string Name;
		uint32_t Width = 1, Height = 1, Samples = 1;
		std::vector<FrameBufferTextureFormat> ColorFormats;
		FrameBufferTextureFormat DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_NONE;
		RenderGraph::AttachmentSpecification GetSpecification() const;
	};

	struct RenderPipelineResourceAsset
	{
		std::filesystem::path Path;
		std::string Type;
	};

	struct RenderPipelinePassAsset
	{
		std::string Name, Type;
		RenderPipelineSettings Settings;
	};

	struct GENGINE_API RenderPipelineAsset
	{
		uint32_t Version = 1;
		std::string Type = "GEngine.Composable";
		RenderPipelineSettings Settings;
		std::vector<RenderPipelineTargetAsset> Targets;
		std::map<std::string, RenderPipelineResourceAsset> Resources;
		std::vector<RenderPipelinePassAsset> Passes;
		std::vector<CameraStackSpecification> CameraStacks;
		std::vector<PostProcessStackSpecification> PostProcessStacks;

		void ValidateStructure() const;
		const RenderPipelineTargetAsset& GetTarget(const std::string& name) const;
		const RenderPipelineResourceAsset& GetResource(const std::string& name) const;
		const CameraStackSpecification& GetCameraStack(const std::string& name) const;
		const PostProcessStackSpecification& GetPostProcessStack(const std::string& name) const;
		std::string SerializePayload() const;
		static RenderPipelineAsset ParsePayload(const std::string& payload);
		static RenderPipelineAsset Load(const std::filesystem::path& path);
		void Save(const std::filesystem::path& path) const;
	};

	class GENGINE_API RenderPipelineAssetContext final
	{
	public:
		const RenderPipelineAsset& GetAsset() const { return m_Asset; }
		RenderGraph::TargetHandle GetTarget(const std::string& name) const;
	private:
		explicit RenderPipelineAssetContext(const RenderPipelineAsset& asset) : m_Asset(asset) {}
		RenderPipelineAsset m_Asset;
		std::map<std::string, RenderGraph::TargetHandle> m_Targets;
		friend class RenderPipelineAssetInstance;
	};

	using RenderPipelinePassValidator = std::function<void(const RenderPipelinePassAsset&,
		const RenderPipelineAsset&, const GraphicsCapabilities&)>;
	using RenderPipelinePassFactory = std::function<Ref<RenderPipelinePass>(const RenderPipelinePassAsset&,
		const Ref<const RenderPipelineAssetContext>&)>;

	class RenderPipelineAssetInstance;
	class GENGINE_API RenderPipelineAssetRegistry final
	{
	public:
		void Register(const std::string& type, RenderPipelinePassValidator validate, RenderPipelinePassFactory create);
		Ref<RenderPipelineAssetInstance> Create(RenderPipelineAsset asset) const;
	private:
		struct Entry { RenderPipelinePassValidator Validate; RenderPipelinePassFactory Create; };
		std::map<std::string, Entry> m_Entries;
	};

	class GENGINE_API RenderPipelineAssetInstance final : public RenderPipeline
	{
	public:
		const RenderPipelineAsset& GetAsset() const { return m_Asset; }
		RenderGraph::TargetHandle GetTarget(const std::string& name) const;
	private:
		RenderPipelineAssetInstance(RenderPipelineAsset asset, std::vector<RenderPipelinePassFactory> factories);
		void Configure(RenderPipelineBuilder& builder) override;
		RenderPipelineAsset m_Asset;
		std::vector<RenderPipelinePassFactory> m_Factories;
		Ref<const RenderPipelineAssetContext> m_Context;
		friend class RenderPipelineAssetRegistry;
	};

	class GENGINE_API RenderPipelineSelector final
	{
	public:
		RenderPipelineSelector() = default;
		RenderPipelineSelector(const RenderPipelineSelector&) = delete;
		RenderPipelineSelector& operator=(const RenderPipelineSelector&) = delete;
		void Activate(const RenderPipelineAsset& asset, const RenderPipelineAssetRegistry& registry);
		void Activate(const std::filesystem::path& path, const RenderPipelineAssetRegistry& registry);
		void Execute(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras);
		void Present(const Ref<CommandBuffer>& command, std::vector<CameraComponent>& cameras);
		void Clear();
		Ref<const RenderPipelineAssetInstance> GetActive() const { return m_Active; }
	private:
		void ActivateCandidate(const RenderPipelineAsset& asset, const RenderPipelineAssetRegistry& registry);
		enum class State { Idle, Activating, Executing, AwaitingPresent, Presenting, Failed };
		State m_State = State::Idle;
		Ref<RenderPipelineAssetInstance> m_Active;
	};
}

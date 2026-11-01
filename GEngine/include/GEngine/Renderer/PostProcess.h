#pragma once
#include "GEngine/Renderer/RenderPipeline.h"
#include <functional>

namespace GEngine
{
	class RenderPipelineAssetContext;
	class RenderPipelineAssetRegistry;
	struct RenderPipelineAsset;
	class Material;
	class GraphicsPipeline;

	enum class PostProcessEffectType { Exposure, Reinhard };
	enum class PostProcessEncoding { Linear, SRGB };
	struct PostProcessEffect
	{
		std::string ID;
		PostProcessEffectType Type = PostProcessEffectType::Exposure;
		bool Enabled = true;
		float EV = 0;
	};
	struct GENGINE_API PostProcessSettings
	{
		bool Enabled = true;
		PostProcessEncoding Encoding = PostProcessEncoding::Linear;
		std::vector<PostProcessEffect> Effects;
		void Validate() const;
	};
	struct GENGINE_API PostProcessStackSpecification
	{
		std::string Name, Input, Output, Shader;
		PostProcessSettings Settings;
		void Validate(const RenderPipelineAsset& asset) const;
	};
	using PostProcessSettingsProvider = std::function<PostProcessSettings(RenderPipelineContext&)>;
	class GENGINE_API PostProcessStackPass final : public RenderPipelinePass
	{
	public:
		PostProcessStackPass(std::string name, PostProcessStackSpecification specification,
			Ref<const RenderPipelineAssetContext> context, PostProcessSettingsProvider settings = {});
		const std::string& GetName() const override { return m_Name; }
		void Configure(RenderPipelineBuilder& builder) override;
		void Prepare(const RenderGraph& graph) override;
		void Execute(RenderPipelineContext& context) override;
		const std::vector<RenderGraph::TargetHandle>& GetIntermediateTargets() const { return m_Intermediate; }
	private:
		void Apply(const PostProcessSettings& settings);
		std::string m_Name;
		PostProcessStackSpecification m_Specification;
		Ref<const RenderPipelineAssetContext> m_Context;
		PostProcessSettingsProvider m_Settings;
		std::vector<RenderGraph::TargetHandle> m_Sources, m_Intermediate;
		std::vector<Ref<Material>> m_Materials;
		std::vector<Ref<GraphicsPipeline>> m_Pipelines;
	};
	GENGINE_API void RegisterPostProcessPass(RenderPipelineAssetRegistry& registry, PostProcessSettingsProvider settings = {});
}

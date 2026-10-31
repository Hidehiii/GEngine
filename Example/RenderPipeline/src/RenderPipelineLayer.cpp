#include "RenderPipelineLayer.h"

#include <GEngine/Graphics/GraphicsPresent.h>
#include <GEngine/Graphics/Sampler.h>
#include <GEngine/Graphics/Texture.h>
#include <GEngine/Graphics/VertexBuffer.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace
{
	using namespace GEngine;

	class UpdateColorPass final : public RenderPipelinePass
	{
	public:
		UpdateColorPass(const Ref<StorageBuffer>& color, RenderGraph::ResourceVersion version)
			: m_Color(color), m_ColorVersion(version)
		{
		}

		const std::string& GetName() const override
		{
			return mName;
		}

		void Configure(RenderPipelineBuilder& builder) override
		{
			auto material = Material::Create(
				Shader::Create("Assets/Shaders/RenderPipelineColor.gasset"), "RenderPipelineColor");
			material->SetResource("ColorData", m_Color);
			m_ComputePipeline = Graphics::GetRenderDevice().CreateComputePipeline(material);

			auto pass = builder.AddComputePass("UpdateColor",
				[this](const Ref<CommandBuffer>& command)
				{
					command->Compute(m_ComputePipeline, 0, 1, 1, 1);
				});
			m_UpdatedVersion = pass.Write(m_ColorVersion,
				RenderGraph::ResourceState::ShaderWrite, GraphicsPipelineStage::Compute);
		}

		void Execute(RenderPipelineContext&) override
		{
		}

		RenderGraph::ResourceVersion GetUpdatedVersion() const
		{
			return m_UpdatedVersion;
		}

	private:
		Ref<StorageBuffer> m_Color;
		Ref<ComputePipeline> m_ComputePipeline;
		RenderGraph::ResourceVersion m_ColorVersion;
		RenderGraph::ResourceVersion m_UpdatedVersion;
		std::string mName = "UpdateColor";
	};

	class GeometryPass final : public RenderPipelinePass
	{
	public:
		GeometryPass(const Ref<StorageBuffer>& color, RenderGraph::TargetHandle target,
			RenderGraph::ResourceVersion inputVersion)
			: m_Color(color), m_Target(target), m_InputVersion(inputVersion)
		{
		}

		const std::string& GetName() const override
		{
			return mName;
		}

		void Configure(RenderPipelineBuilder& builder) override
		{
			auto material = Material::Create(
				Shader::Create("Assets/Shaders/RenderPipelineTriangle.gasset"), "RenderPipelineTriangle");
			material->SetResource("ColorData", m_Color);

			const float vertices[] =
			{
				 0.0f,  0.65f, 0.0f,
				 0.65f, -0.55f, 0.0f,
				-0.65f, -0.55f, 0.0f
			};
			auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
			m_Pipeline = Graphics::GetRenderDevice().CreateGraphicsPipeline(material, vertexBuffer);

			auto pass = builder.AddGraphicsPass("Geometry", m_Target,
				[this](const Ref<CommandBuffer>& command)
				{
					command->Render(m_Pipeline, 0);
				});
			pass.Read(m_InputVersion, RenderGraph::ResourceState::ShaderWrite,
				GraphicsPipelineStage::FragmentShader);
		}

		void Execute(RenderPipelineContext&) override
		{
		}

	private:
		Ref<StorageBuffer> m_Color;
		RenderGraph::TargetHandle m_Target;
		RenderGraph::ResourceVersion m_InputVersion;
		Ref<GraphicsPipeline> m_Pipeline;
		std::string mName = "Geometry";
	};

	class PresentPass final : public RenderPipelinePass
	{
	public:
		explicit PresentPass(RenderGraph::TargetHandle target)
			: m_Target(target)
		{
		}

		const std::string& GetName() const override
		{
			return mName;
		}

		void Configure(RenderPipelineBuilder&) override
		{
		}

		void Execute(RenderPipelineContext&) override
		{
		}

		void Present(RenderPipelineContext& context) override
		{
			if (!m_Pipeline)
			{
				const auto targetTexture = context.Graph.GetTexture2D(
					context.Graph.GetColorAttachment(m_Target));
				if (!targetTexture)
					throw std::runtime_error("RenderPipeline present target was not created.");

				auto material = Material::Create(
					Shader::Create("Assets/Shaders/RenderPipelinePresent.gasset"), "RenderPipelinePresent");
				material->SetConstant<Matrix4x4>("GE_MATRIX_M", Matrix4x4(1.0f));
				material->SetConstant<Vector4>("GE_IMAGE_COLOR", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
				material->SetConstant<Vector2>("GE_IMAGE_TILING", Vector2{1.0f, 1.0f});
				material->SetResource<Texture2D>("GE_IMAGE_TEXTURE", targetTexture);
				material->SetResource<Sampler>("GE_IMAGE_SAMPLER", Sampler::GetDefaultSampler());

				const float vertices[][5] =
				{
					{-0.5f, -0.5f, 0.0f, 0.0f, 0.0f},
					{ 0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
					{ 0.5f,  0.5f, 0.0f, 1.0f, 1.0f},
					{-0.5f,  0.5f, 0.0f, 0.0f, 1.0f}
				};
				const uint32_t indices[] = {0, 1, 2, 2, 3, 0};
				auto& device = Graphics::GetRenderDevice();
				auto vertexBuffer = device.CreateVertexBuffer(vertices, sizeof(vertices), 0,
					VERTEX_TOPOLOGY_TRIANGLE);
				vertexBuffer->SetIndexBuffer(device.CreateIndexBuffer(indices, 6));
				m_Pipeline = device.CreateGraphicsPipeline(material, vertexBuffer);
			}
			GraphicsPresent::Render(m_Pipeline, 0);
		}

	private:
		RenderGraph::TargetHandle m_Target;
		Ref<GraphicsPipeline> m_Pipeline;
		std::string mName = "Present";
	};
}

namespace GEngine
{
	void ForwardRenderPipeline::Configure(RenderPipelineBuilder& builder)
	{
		RenderGraph::AttachmentSpecification attachment{};
		attachment.ColorFormats = {FRAME_BUFFER_TEXTURE_FORMAT_RGBA8};
		attachment.DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
		m_Target = builder.CreateRenderTarget("PipelineColor", attachment, 512, 512);

		m_Color = Graphics::GetRenderDevice().CreateStorageBuffer(16);
		const float initialColor[] = {0.95f, 0.45f, 0.12f, 1.0f};
		m_Color->SetData(sizeof(initialColor), initialColor);

		const auto colorResource = builder.GetGraph().ImportStorageBuffer(
			"PipelineColorData", m_Color, RenderGraph::ResourceState::ShaderWrite);
		const auto colorVersion = builder.GetGraph().GetVersion(colorResource);

		auto updatePass = CreateRef<UpdateColorPass>(m_Color, colorVersion);
		AddPass(updatePass);
		updatePass->Configure(builder);

		auto geometryPass = CreateRef<GeometryPass>(m_Color, m_Target, updatePass->GetUpdatedVersion());
		AddPass(geometryPass);
		geometryPass->Configure(builder);

		AddPass(CreateRef<PresentPass>(m_Target));
	}

	RenderPipelineLayer::RenderPipelineLayer()
		: Layer("RenderPipeline")
	{
	}

	void RenderPipelineLayer::OnAttach()
	{
		CameraComponent camera;
		camera.SetPerspective(60.0f, 0.1f, 100.0f);
		camera.SetViewportSize(1280, 720);
		camera.isPrimary = true;
		m_Cameras.push_back(camera);
		GE_INFO("RenderPipeline initialized: compute, offscreen geometry, sampled presentation.");
	}

	void RenderPipelineLayer::OnRender()
	{
		m_Pipeline.Execute(GraphicsPresent::GetCommandBuffer(), m_Cameras);
		if (m_FrameCount == 118)
		{
			float result[4]{};
			m_Pipeline.GetColor()->ReadData(sizeof(result), result);
			if (std::abs(result[0] - 0.15f) > 0.001f || std::abs(result[1] - 0.45f) > 0.001f ||
				std::abs(result[2] - 0.12f) > 0.001f || std::abs(result[3] - 1.0f) > 0.001f)
				throw std::runtime_error("RenderPipeline compute readback mismatch.");
			GE_INFO("RenderPipeline compute readback passed after 119 frames.");
		}
	}

	void RenderPipelineLayer::OnPresent()
	{
		m_Pipeline.Present(GraphicsPresent::GetCommandBuffer(), m_Cameras);
		++m_FrameCount;

		static const uint64_t frameLimit = []
		{
			const char* value = std::getenv("GENGINE_RENDER_PIPELINE_FRAME_LIMIT");
			return value ? std::strtoull(value, nullptr, 10) : 0ull;
		}();
		if (frameLimit && m_FrameCount >= frameLimit)
		{
			GE_INFO("RenderPipeline completed {} frames; requesting normal shutdown.", m_FrameCount);
			Application::Get().Close();
		}
	}
}

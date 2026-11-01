#include "RenderPipelineLayer.h"
#include "CameraStackChecks.h"
#include "PostProcessChecks.h"

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

	template<typename Callback>
	void ExpectLifecycleRejection(Callback&& callback)
	{
		try { callback(); }
		catch (const std::logic_error&) { return; }
		throw std::runtime_error("RenderPipeline accepted an invalid lifecycle operation.");
	}

	class LifecycleCheckPass final : public RenderPipelinePass
	{
	public:
		explicit LifecycleCheckPass(RenderPipeline& owner) : m_Owner(owner) {}
		const std::string& GetName() const override { return m_Name; }
		void Configure(RenderPipelineBuilder&) override
		{
			if (m_FailFirstConfiguration)
			{
				m_FailFirstConfiguration = false;
				throw std::runtime_error("Intentional pipeline configuration failure.");
			}
			m_Frame = 0;
			ExpectLifecycleRejection([this] { m_Owner.Reset(); });
			ExpectLifecycleRejection([this] { m_Owner.RemovePass(m_Name); });
			ExpectLifecycleRejection([this] { m_Owner.AddPass(nullptr); });
			GE_INFO("RenderPipeline external pass configured; mutation guards passed.");
		}
		void Execute(RenderPipelineContext& context) override
		{
			if (context.Frame != m_Frame)
				throw std::runtime_error("RenderPipeline Execute frame mismatch.");
			ExpectLifecycleRejection([this] { m_Owner.Reset(); });
			ExpectLifecycleRejection([this, &context] { m_Owner.Execute(context.Command, context.Cameras); });
			ExpectLifecycleRejection([this, &context] { m_Owner.Present(context.Command, context.Cameras); });
		}
		void Present(RenderPipelineContext& context) override
		{
			if (context.Frame != m_Frame)
				throw std::runtime_error("RenderPipeline Present frame mismatch.");
			ExpectLifecycleRejection([this] { m_Owner.Reset(); });
			++m_Frame;
		}
	private:
		RenderPipeline& m_Owner;
		std::string m_Name = "LifecycleChecks";
		uint64_t m_Frame = 0;
		bool m_FailFirstConfiguration = true;
	};

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
			const Ref<UpdateColorPass>& producer)
			: m_Color(color), m_Target(target), m_Producer(producer)
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
			pass.Read(m_Producer->GetUpdatedVersion(), RenderGraph::ResourceState::ShaderWrite,
				GraphicsPipelineStage::FragmentShader);
		}

		void Execute(RenderPipelineContext&) override
		{
		}

	private:
		Ref<StorageBuffer> m_Color;
		RenderGraph::TargetHandle m_Target;
		Ref<UpdateColorPass> m_Producer;
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
		m_Target = builder.CreateRenderTarget("PipelineColor", attachment, m_Width, m_Height);

		m_Color = Graphics::GetRenderDevice().CreateStorageBuffer(16);
		const float initialColor[] = {0.95f, 0.45f, 0.12f, 1.0f};
		m_Color->SetData(sizeof(initialColor), initialColor);

		const auto colorResource = builder.GetGraph().ImportStorageBuffer(
			"PipelineColorData", m_Color, RenderGraph::ResourceState::ShaderWrite);
		const auto colorVersion = builder.GetGraph().GetVersion(colorResource);

		auto updatePass = CreateRef<UpdateColorPass>(m_Color, colorVersion);
		AddPass(updatePass);

		auto geometryPass = CreateRef<GeometryPass>(m_Color, m_Target, updatePass);
		AddPass(geometryPass);

		AddPass(CreateRef<PresentPass>(m_Target));
	}

	void ForwardRenderPipeline::Resize(uint32_t width, uint32_t height)
	{
		if (!width || !height)
			throw std::invalid_argument("RenderPipeline dimensions must be nonzero.");
		Reset();
		m_Width = width;
		m_Height = height;
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
		if (std::getenv("GENGINE_RENDER_PIPELINE_POST_CHECK"))
		{
			m_Cameras.resize(2);
			for (auto& view : m_Cameras) view.SetOrthoGraphic(2,-1,1);
			m_AssetChecks = CreatePostProcessChecks();
			return;
		}
		if (std::getenv("GENGINE_RENDER_PIPELINE_STACK_CHECK"))
		{
			m_Cameras.resize(3);
			for (auto& view : m_Cameras) view.SetOrthoGraphic(2,-1,1);
			m_AssetChecks = CreateCameraStackChecks();
			return;
		}
		if (std::getenv("GENGINE_RENDER_PIPELINE_ASSET_CHECK"))
		{
			m_AssetChecks = CreatePipelineAssetChecks();
			return;
		}
		m_LifecycleChecks = CreateRef<LifecycleCheckPass>(m_Pipeline);
		m_Pipeline.AddPass(m_LifecycleChecks);
		ExpectLifecycleRejection([this] { m_Pipeline.AddPass(m_LifecycleChecks); });
		m_Pipeline.RemovePass(m_LifecycleChecks->GetName());
		ExpectLifecycleRejection([this] { m_Pipeline.RemovePass(m_LifecycleChecks->GetName()); });
		m_Pipeline.AddPass(m_LifecycleChecks);
		GE_INFO("RenderPipeline initialized: compute, offscreen geometry, sampled presentation.");
	}

	void RenderPipelineLayer::OnRender()
	{
		if (m_AssetChecks) { m_AssetChecks->Execute(m_FrameCount, m_Cameras); return; }
		const auto command = GraphicsPresent::GetCommandBuffer();
		if (m_FrameCount == 0)
		{
			ExpectLifecycleRejection([&] { m_Pipeline.Present(command, m_Cameras); });
			ExpectLifecycleRejection([&] { m_Pipeline.Execute(nullptr, m_Cameras); });
			bool configurationFailed = false;
			try { m_Pipeline.Execute(command, m_Cameras); }
			catch (const std::runtime_error& error)
			{
				if (std::string(error.what()) != "Intentional pipeline configuration failure.") throw;
				configurationFailed = true;
			}
			if (!configurationFailed)
				throw std::runtime_error("RenderPipeline failed to configure an external pass.");
			ExpectLifecycleRejection([&] { m_Pipeline.Execute(command, m_Cameras); });
			m_Pipeline.Reset();
			m_Pipeline.AddPass(m_LifecycleChecks);
			GE_INFO("RenderPipeline configuration failure recovery passed.");
		}
		if (m_FrameCount == 30 || m_FrameCount == 60)
		{
			m_Pipeline.Resize(m_FrameCount == 30 ? 256 : 640, m_FrameCount == 30 ? 384 : 360);
			m_GenerationFrames = 0;
			m_Pipeline.AddPass(m_LifecycleChecks);
			GE_INFO("RenderPipeline rebuilt offscreen target at frame {}.", m_FrameCount);
		}
		m_Pipeline.Execute(GraphicsPresent::GetCommandBuffer(), m_Cameras);
		ExpectLifecycleRejection([&] { m_Pipeline.Execute(command, m_Cameras); });
		ExpectLifecycleRejection([this] { m_Pipeline.Reset(); });
		ExpectLifecycleRejection([this] { m_Pipeline.RemovePass(m_LifecycleChecks->GetName()); });
		++m_GenerationFrames;
		const auto texture = m_Pipeline.GetGraph().GetTexture2D(
			m_Pipeline.GetGraph().GetColorAttachment(m_Pipeline.GetTarget()));
		const uint32_t width = m_FrameCount < 30 ? 512 : (m_FrameCount < 60 ? 256 : 640);
		const uint32_t height = m_FrameCount < 30 ? 512 : (m_FrameCount < 60 ? 384 : 360);
		if (!texture || texture->GetWidth() != width || texture->GetHeight() != height)
			throw std::runtime_error("RenderPipeline rebuilt target dimension mismatch.");
		if (m_FrameCount == 28 || m_FrameCount == 58 || m_FrameCount == 118)
		{
			float result[4]{};
			m_Pipeline.GetColor()->ReadData(sizeof(result), result);
			const float expectedRed = m_GenerationFrames % 2 ? 0.15f : 0.95f;
			if (std::abs(result[0] - expectedRed) > 0.001f || std::abs(result[1] - 0.45f) > 0.001f ||
				std::abs(result[2] - 0.12f) > 0.001f || std::abs(result[3] - 1.0f) > 0.001f)
				throw std::runtime_error("RenderPipeline compute readback mismatch.");
			GE_INFO("RenderPipeline compute readback passed at frame {} ({}x{}).", m_FrameCount + 1, width, height);
		}
	}

	void RenderPipelineLayer::OnPresent()
	{
		if (m_AssetChecks) m_AssetChecks->Present(m_Cameras);
		else
		{
			m_Pipeline.Present(GraphicsPresent::GetCommandBuffer(), m_Cameras);
			ExpectLifecycleRejection([this] { m_Pipeline.AddPass(m_LifecycleChecks); });
			ExpectLifecycleRejection([this] { m_Pipeline.Present(GraphicsPresent::GetCommandBuffer(), m_Cameras); });
		}
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

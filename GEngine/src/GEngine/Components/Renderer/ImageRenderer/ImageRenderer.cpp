#include "GEpch.h"
#include "GEngine/Components/Renderer/ImageRenderer/ImageRenderer.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Components/Transform/Transform.h"
#include <stdexcept>

namespace GEngine
{
	ImageRenderer::ImageRenderer(const ImageRenderer& other)
		: Component(other), m_Color(other.m_Color), m_Texture(other.m_Texture),
		m_Tiling(other.m_Tiling), m_Material(other.m_Material)
	{
	}

	void ImageRenderer::SetMaterial(const Ref<Material>& material)
	{
		if (!material)
			throw std::invalid_argument("ImageRenderer requires a material.");
		m_Material = material;
		m_Pipeline.reset();
	}

	void ImageRenderer::OnRender()
	{
		if (m_Texture)
			throw std::logic_error("Textured ImageRenderer drawing is unsupported.");
		if (!m_Material)
			throw std::logic_error("ImageRenderer requires a material before rendering.");
		if (!m_Pipeline)
		{
			const float vertices[][3] = {
				{ -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f },
				{ 0.5f,  0.5f, 0.0f }, { -0.5f,  0.5f, 0.0f }
			};
			const uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
			auto buffer = VertexBuffer::Create(vertices, sizeof(vertices));
			buffer->SetIndexBuffer(IndexBuffer::Create(indices, 6));
			m_Pipeline = GraphicsPipeline::Create(m_Material, buffer);
		}
		m_Material->SetConstant<Matrix4x4>("GE_MATRIX_M", m_GameObject.GetComponent<Transform>().GetModelMatrix());
		m_Material->SetConstant<Vector4>("GE_IMAGE_COLOR", m_Color);
		GraphicsPresent::Render(m_Pipeline, 0);
	}
}

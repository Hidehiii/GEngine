#include "TriangleLayer.h"

namespace GEngine
{
	TriangleLayer::TriangleLayer()
		: Layer("Triangle")
	{
	}

	void TriangleLayer::OnAttach()
	{
		const float vertices[] =
		{
			 0.0f,  0.65f, 0.0f,
			 0.65f, -0.55f, 0.0f,
			-0.65f, -0.55f, 0.0f
		};

		auto shader = Shader::Create("Assets/Shaders/Triangle.shader");
		auto material = Material::Create(shader, "TriangleMaterial");

		auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		m_Pipeline = GraphicsPipeline::Create(material, vertexBuffer);
	}

	void TriangleLayer::OnPresent()
	{
		GraphicsPresent::Render(m_Pipeline, 0);
	}
}

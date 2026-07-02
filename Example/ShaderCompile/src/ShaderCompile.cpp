#include "ShaderCompile.h"

namespace GEngine
{
	ShaderCompile::ShaderCompile()
	{
	}
	void ShaderCompile::OnAttach()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/ExampleShader.shader");
		Ref<Material> material = Material::Create(shader, "ExampleMaterial");

		float trianglePos[] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
		};
		uint32_t triangleIndices[] = {
			0, 1, 2
		};

		m_TrianglePipeline = GraphicsPipeline::Create(
			material,
			VertexBuffer::Create(sizeof(float) * _countof(trianglePos))
		);
		m_TrianglePipeline->GetVertexBuffer()->SetLayout({
			{ SHADER_INPUT_DATA_TYPE_FLOAT3, "in_Position" },
			});
		m_TrianglePipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(triangleIndices, _countof(triangleIndices)));
		m_TrianglePipeline->GetVertexBuffer()->SetData(trianglePos, sizeof(float) * _countof(trianglePos));

		m_TrianglePipeline->GetMaterial()->SetConstant<Vector4>("_Color", Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		m_TrianglePipeline->GetMaterial()->SetConstant<Vector4>("_Color1", Vector4(0.0f, 0.5f, 0.0f, 1.0f));
	}
	void ShaderCompile::OnPresent()
	{
		GraphicsPresent::Render(m_TrianglePipeline, 0);
	}
	void ShaderCompile::OnRender()
	{
	}
	void ShaderCompile::OnUpdate()
	{
		
	}
	void ShaderCompile::OnImGuiRender()
	{
	}
	void ShaderCompile::OnEvent(Event& e)
	{
		//GE_INFO("Event: {0}", e.ToString());
	}
}
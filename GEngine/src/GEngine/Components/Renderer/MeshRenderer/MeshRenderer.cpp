#include "GEpch.h"
#include "MeshRenderer.h"
#include "GEngine/Components/3D/MeshFilter/MeshFilter.h"
#include "GEngine/Components/Transform/Transform.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	void MeshRenderer::OnRender()
	{
		if (m_Pipeline)
		{
			m_Material->SetConstant<Matrix4x4>("GE_MATRIX_M", m_GameObject.GetComponent<Transform>().GetModelMatrix());
			//m_Pipeline->Render();
		}
		else
		{
			UpdatePipeline();
		}
	}
	void MeshRenderer::SetMaterial(Ref<Material> material)
	{
		if(m_Pipeline)
			m_Pipeline->SetMaterial(material);
		m_Material = material;
	}
	void MeshRenderer::UpdatePipeline()
	{
		if (m_GameObject.HasComponent<MeshFilter>() && m_GameObject.GetComponent<MeshFilter>().m_Mesh)
		{
			Mesh mesh = m_GameObject.GetComponent<MeshFilter>().m_Mesh;
			if (m_Pipeline)
			{
				m_Pipeline = GraphicsPipeline::Create(
					m_Pipeline->GetMaterial(),
					VertexBuffer::Create(mesh.m_Vertices.size() * sizeof(Vertex))
				);
				m_Material = m_Pipeline->GetMaterial();
			}
			else
			{
				if (m_Material == nullptr)
				{
					m_Material = Material::Create(Shader::Create("Assets/Shaders/3D/Default.glsl"), "Default");
				}
				m_Pipeline = GraphicsPipeline::Create(
					m_Material,
					VertexBuffer::Create(mesh.m_Vertices.size() * sizeof(Vertex))
				);
			}
			m_Pipeline->GetVertexBuffer()->SetLayout({
				{SHADER_INPUT_DATA_TYPE_FLOAT4, "PositionOS"	, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT4, "Color"			, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT4, "Normal"		, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT3, "Tangent"		, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT2, "UV0"			, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT2, "UV1"			, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT2, "UV2"			, false},
				{SHADER_INPUT_DATA_TYPE_FLOAT2, "UV3"			, false},
				});
			m_Pipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(mesh.m_Indices.data(), mesh.m_Indices.size()));
			m_Pipeline->GetVertexBuffer()->SetVertexData(mesh.m_Vertices.data(), mesh.m_Vertices.size() * sizeof(Vertex));
		}
	}
}


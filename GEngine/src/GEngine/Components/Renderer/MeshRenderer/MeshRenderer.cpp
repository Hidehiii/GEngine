#include "GEpch.h"
#include "MeshRenderer.h"
#include "GEngine/Components/3D/MeshFilter/MeshFilter.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
	void MeshRenderer::OnRender()
	{
		if (m_Pipeline)
		{
			Renderer::SetModelUniforms(m_GameObject.GetComponent<Transform>());
			m_Pipeline->Render();
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
					m_Material = Material::Create(Shader::Create("Assets/Shaders/3D/Default.glsl"));
				}
				m_Pipeline = GraphicsPipeline::Create(
					m_Material,
					VertexBuffer::Create(mesh.m_Vertices.size() * sizeof(Vertex))
				);
			}
			m_Pipeline->GetVertexBuffer()->SetLayout({
				{ShaderDataType::float4, "PositionOS"	},
				{ShaderDataType::float4, "Color"		},
				{ShaderDataType::float4, "Normal"		},
				{ShaderDataType::float3, "Tangent"		},
				{ShaderDataType::float2, "UV0"			},
				{ShaderDataType::float2, "UV1"			},
				{ShaderDataType::float2, "UV2"			},
				{ShaderDataType::float2, "UV3"			},
				});
			m_Pipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(mesh.m_Indices.data(), mesh.m_Indices.size()));
			m_Pipeline->GetVertexBuffer()->SetData(mesh.m_Vertices.data(), mesh.m_Vertices.size() * sizeof(Vertex));
		}
	}
}


#include "GEpch.h"
#include "MeshRenderer.h"
#include "GEngine/Components/3D/MeshFilter/MeshFilter.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Renderer/Renderer.h"

namespace GEngine
{
	void MeshRenderer::Init()
	{
		if (m_GameObject.HasComponent<MeshFilter>() && m_GameObject.GetComponent<MeshFilter>().GetMesh())
		{
			Mesh mesh = m_GameObject.GetComponent<MeshFilter>().GetMesh();
			m_VertexBuffer = (VertexBuffer::Create(mesh.m_Vertices.size() * sizeof(Vertex)));
			m_VertexBuffer->SetLayout({
				{ShaderDataType::float4, "PositionOS"},
				{ShaderDataType::float4, "Color"},
				{ShaderDataType::float4, "Normal"}
				});

			m_VertexBuffer->SetIndexBuffer(IndexBuffer::Create(mesh.m_Indices.data(), mesh.m_Indices.size()));
			if(m_Material == nullptr)
				m_Material = Material::Create(Shader::Create("Assets/Shaders/3D/Default.glsl"));
		}
		else
		{
			GE_CORE_ERROR("{0} : There is no mesh filter attached to the game object or the mesh filter has no mesh attached to it.", m_GameObject.GetComponent<Attribute>().m_Name);

			m_VertexBuffer = nullptr;
		}
	}
	void MeshRenderer::OnRender()
	{
		if (m_Material && m_VertexBuffer)
		{
			m_VertexBuffer->Bind();
			m_Material->UploadData();
			m_VertexBuffer->SetData(m_GameObject.GetComponent<MeshFilter>().GetMesh().m_Vertices.data(), m_GameObject.GetComponent<MeshFilter>().GetMesh().m_Vertices.size() * sizeof(Vertex));
			Renderer::SetModelUniforms(m_GameObject.GetComponent<Transform>());
			//RenderCommand::DrawTriangles(m_VertexBuffer);
		}
		else
		{
			GE_CORE_ERROR("{0} :There is no vertex array attached to the mesh renderer.", m_GameObject.GetComponent<Attribute>().m_Name);
		}
	}
	void MeshRenderer::SetMaterial(Ref<Material> material)
	{
		m_Material = material;
	}
}


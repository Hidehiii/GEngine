#include "GEpch.h"
#include "GEngine/Components/Renderer/MeshRenderer/MeshRenderer.h"
#include "GEngine/Components/3D/MeshFilter/MeshFilter.h"
#include "GEngine/Components/Transform/Transform.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Graphics/CommandBuffer.h"
#include <stdexcept>

namespace GEngine
{
	MeshRenderer::MeshRenderer(const MeshRenderer& other)
		: Component(other), m_Material(other.m_Material)
	{
	}

	void MeshRenderer::OnRender()
	{
		OnRender(GraphicsPresent::GetCommandBuffer());
	}

	void MeshRenderer::OnRender(const Ref<CommandBuffer>& command)
	{
		if (!command) throw std::invalid_argument("MeshRenderer requires a command buffer.");
		if (!m_Material)
			throw std::logic_error("MeshRenderer requires a material before rendering.");
		if (!m_Pipeline)
			UpdatePipeline();
		if (!m_Pipeline)
			throw std::logic_error("MeshRenderer requires a nonempty MeshFilter before rendering.");
		m_Material->SetConstant<Matrix4x4>("GE_MATRIX_M", m_GameObject.GetComponent<Transform>().GetModelMatrix());
		command->Render(m_Pipeline, 0);
	}
	void MeshRenderer::SetMaterial(Ref<Material> material)
	{
		if (!material)
			throw std::invalid_argument("MeshRenderer requires a material.");
		m_Material = material;
		m_Pipeline.reset();
	}
	void MeshRenderer::UpdatePipeline()
	{
		m_Pipeline.reset();
		if (!m_Material || !m_GameObject.HasComponent<MeshFilter>())
			return;
		const Mesh& mesh = m_GameObject.GetComponent<MeshFilter>().m_Mesh;
		if (!mesh)
			return;
		auto buffer = VertexBuffer::Create(mesh.m_Vertices.data(),
			static_cast<uint32_t>(mesh.m_Vertices.size() * sizeof(Vertex)));
		buffer->SetIndexBuffer(IndexBuffer::Create(mesh.m_Indices.data(), static_cast<uint32_t>(mesh.m_Indices.size())));
		m_Pipeline = GraphicsPipeline::Create(m_Material, buffer);
	}
}


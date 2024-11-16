#include "GEpch.h"
#include "MeshFilter.h"
#include "GEngine/Components/Renderer/MeshRenderer/MeshRenderer.h"

namespace GEngine
{
	void MeshFilter::SetMesh(const Mesh& mesh)
	{
		m_Mesh = mesh;
		if (m_GameObject.HasComponent<MeshRenderer>())
		{
			m_GameObject.GetComponent<MeshRenderer>().UpdateMesh(mesh);
		}
	}
}
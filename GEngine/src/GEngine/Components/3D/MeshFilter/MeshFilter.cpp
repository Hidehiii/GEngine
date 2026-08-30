#include "GEpch.h"
#include "GEngine/Components/3D/MeshFilter/MeshFilter.h"
#include "GEngine/Components/Renderer/MeshRenderer/MeshRenderer.h"

namespace GEngine
{
	void MeshFilter::SetMesh(const Mesh& mesh)
	{
		m_Mesh = mesh;
		if (m_GameObject.HasComponent<MeshRenderer>())
		{
			m_GameObject.GetComponent<MeshRenderer>().UpdatePipeline();
		}
	}
}
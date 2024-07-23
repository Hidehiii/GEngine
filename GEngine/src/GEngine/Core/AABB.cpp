#include "GEpch.h"
#include "AABB.h"

#include "GEngine/Components/3D/MeshFilter/MeshFilter.h"

namespace GEngine
{
	void AABB::CalculateAABB(GameObject& gameObject)
	{
		// Get the mesh filter component of the game object
		MeshFilter meshFilter = gameObject.GetComponent<MeshFilter>();
		if (meshFilter)
		{
			// Get the mesh of the mesh filter
			auto& mesh = meshFilter.GetMesh();
			if (mesh.m_Vertices.size() > 0)
			{
				// Get the first vertex of the mesh
				auto& vertex = mesh.m_Vertices[0];
				m_Min = vertex.m_Position;
				m_Max = vertex.m_Position;

				// Calculate the AABB of the mesh
				for (auto& vertex : mesh.m_Vertices)
				{
					m_Min.value.x = Math::Min({ m_Min.value.x, vertex.m_Position.value.x});
					m_Min.value.y = Math::Min({ m_Min.value.y, vertex.m_Position.value.y});
					m_Min.value.z = Math::Min({ m_Min.value.z, vertex.m_Position.value.z});

					m_Max.value.x = Math::Max({ m_Max.value.x, vertex.m_Position.value.x});
					m_Max.value.y = Math::Max({ m_Max.value.y, vertex.m_Position.value.y});
					m_Max.value.z = Math::Max({ m_Max.value.z, vertex.m_Position.value.z});
				}
			}
		}
	}
}

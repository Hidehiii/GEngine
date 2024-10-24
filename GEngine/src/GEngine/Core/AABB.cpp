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
					m_Min.x = Math::Min({ m_Min.x, vertex.m_Position.x });
					m_Min.y = Math::Min({ m_Min.y, vertex.m_Position.y });
					m_Min.z = Math::Min({ m_Min.z, vertex.m_Position.z });

					m_Max.x = Math::Max({ m_Max.x, vertex.m_Position.x });
					m_Max.y = Math::Max({ m_Max.y, vertex.m_Position.y });
					m_Max.z = Math::Max({ m_Max.z, vertex.m_Position.z });
				}
			}
		}
	}
}

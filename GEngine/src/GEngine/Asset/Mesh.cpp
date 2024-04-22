#include "GEpch.h"
#include "Mesh.h"

namespace GEngine
{
	std::unordered_map<std::string, Mesh> MeshLibrary::m_Meshes = std::unordered_map<std::string, Mesh>();

	bool MeshLibrary::AddMesh(const Mesh& mesh)
	{
		if (m_Meshes.find(mesh.m_Name) != m_Meshes.end())
		{
			GE_CORE_WARN("Mesh with name {0} already exists", mesh.m_Name);
			return false;
		}
		m_Meshes[mesh.m_Name] = mesh;
		return true;
	}
	std::string MeshLibrary::AddMesh_Force(Mesh& mesh)
	{
 		if (m_Meshes.find(mesh.m_Name) != m_Meshes.end())
		{
			GE_CORE_WARN("Mesh with name {0} already exists", mesh.m_Name);
			mesh.m_Name = mesh.m_Name + "_copy";
		}
		m_Meshes[mesh.m_Name] = mesh;
		return mesh.m_Name;
	}
	Mesh MeshLibrary::GetMesh(const std::string& name)
	{
		if (m_Meshes.find(name) != m_Meshes.end())
		{
			return m_Meshes[name];
		}
		return Mesh();
	}
	std::vector<std::string> MeshLibrary::GetMeshNames()
	{
		std::vector<std::string> names;
		for (auto& mesh : m_Meshes)
		{
			names.push_back(mesh.first);
		}
		return names;
	}
}
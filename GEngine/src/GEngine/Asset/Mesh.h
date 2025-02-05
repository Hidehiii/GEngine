#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include <unordered_map>
#include <filesystem>
#include <string>
#include <xhash>


namespace GEngine
{
	struct Vertex
	{
		Vertex() = default;

		Vector4 m_Position	= { 0.0f, 0.0f, 0.0f, 1.0f };
		Vector4 m_Color		= { 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 m_Normal	= { 0.0f, 0.0f, -1.0f, 0.0f };
		Vector3 m_Tangent	= { 1.0f, 0.0f, 0.0f };
		Vector2 m_UV0		= { 0.0f, 0.0f };
		Vector2 m_UV1		= { 0.0f, 0.0f };
		Vector2 m_UV2		= { 0.0f, 0.0f };
		Vector2 m_UV3		= { 0.0f, 0.0f };

		bool operator==(const Vertex& other) const {
			return m_Position == other.m_Position && m_Color == other.m_Color && m_Normal == other.m_Normal && m_Tangent == other.m_Tangent && 
				m_UV0 == other.m_UV0 && m_UV1 == other.m_UV1 && m_UV2 == other.m_UV2 && m_UV3== other.m_UV3;
		}
	};
}

namespace std
{
	template<>
	struct hash<GEngine::Vertex>
	{
		size_t operator()(const GEngine::Vertex& vertex) const
		{
			return ((((((hash<GEngine::Vector4>()(vertex.m_Position) ^
				(hash<GEngine::Vector4>()(vertex.m_Color) << 1)) >> 1) ^
				(hash<GEngine::Vector4>()(vertex.m_Normal) << 1) >> 1) ^
				(hash<GEngine::Vector3>()(vertex.m_Tangent) << 1) |
				(hash<GEngine::Vector2>()(vertex.m_UV0)) >> 1) << 1) ^
				(hash<GEngine::Vector2>()(vertex.m_UV1)) >> 1) |
				(hash<GEngine::Vector2>()(vertex.m_UV2)) ^ 
				(hash<GEngine::Vector2>()(vertex.m_UV3) << 1);
		}
	};
}

namespace GEngine
{
	class GENGINE_API Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = default;

		operator bool() const { return m_Vertices.size() > 0 && m_Indices.size() > 0; }
	public:
		std::filesystem::path	m_SourceFilePath;
		std::string				m_Name;
		std::vector<Vertex>		m_Vertices;
		std::vector<uint32_t>	m_Indices;
	private:
		std::unordered_map<Vertex, uint32_t> m_VertexsIndex;
		friend class ModelImporter;
	};

	class GENGINE_API MeshLibrary
	{
	public:
		static bool AddMesh(const Mesh& mesh);
		static std::string AddMesh_Force(Mesh& mesh);
		static Mesh GetMesh(const std::string& name);
		static std::vector<std::string> GetMeshNames();
	private:
		static std::unordered_map<std::string, Mesh> m_Meshes;
	};
}





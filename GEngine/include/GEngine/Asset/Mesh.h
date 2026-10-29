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
		Vector4 m_Tangent	= { 1.0f, 0.0f, 0.0f, 0.0f };
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
			size_t seed = 0x9e3779b97f4a7c15ULL;
			const auto combine = [&seed](float value)
			{
				seed ^= std::hash<float>{}(value) + 0x9e3779b9U +
					(seed << 6) + (seed >> 2);
			};
			for (float value : { vertex.m_Position.x, vertex.m_Position.y, vertex.m_Position.z, vertex.m_Position.w,
				vertex.m_Color.x, vertex.m_Color.y, vertex.m_Color.z, vertex.m_Color.w,
				vertex.m_Normal.x, vertex.m_Normal.y, vertex.m_Normal.z, vertex.m_Normal.w,
				vertex.m_Tangent.x, vertex.m_Tangent.y, vertex.m_Tangent.z, vertex.m_Tangent.w,
				vertex.m_UV0.x, vertex.m_UV0.y, vertex.m_UV1.x, vertex.m_UV1.y,
				vertex.m_UV2.x, vertex.m_UV2.y, vertex.m_UV3.x, vertex.m_UV3.y })
			{
				combine(value);
			}
			return seed;
		}
	};
}

namespace GEngine
{
	class GENGINE_API Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh& other)
			: m_SourceFilePath(other.m_SourceFilePath)
			, m_Name(other.m_Name)
			, m_Vertices(other.m_Vertices)
			, m_Indices(other.m_Indices)
			, m_VertexsIndex()
		{
		}

		Mesh& operator=(const Mesh& other)
		{
			if (this == &other) return *this;
			m_SourceFilePath = other.m_SourceFilePath;
			m_Name = other.m_Name;
			m_Vertices = other.m_Vertices;
			m_Indices = other.m_Indices;
			m_VertexsIndex.clear();
			return *this;
		}

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





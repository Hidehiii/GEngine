#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include <unordered_map>
#include <filesystem>
#include <string>

namespace GEngine
{
	struct Vertex
	{
		Vector4 m_Position;
		Vector4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 m_Normal;
		//Vector4 Tangent;
		//Vector4 UV;
	};
	struct Triangle
	{
		Vertex Vertices[3];
	};

	class GENGINE_API Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = default;

		operator bool() const { return m_Vertices.size() > 0 && m_Indices.size() > 0 && m_Triangles.size() > 0; }
	public:
		std::filesystem::path m_SourceFilePath;
		std::string m_Name;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<Triangle> m_Triangles;
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



#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Asset/Mesh.h"
#include <fbxsdk.h>

namespace GEngine
{
	class GENGINE_API ModelImporter
	{
	public:
        ModelImporter() = default;
		ModelImporter(const ModelImporter&) = default;
		~ModelImporter();

		bool LoadMesh(const std::string filaName);

		Mesh GetMesh(int index) const { return m_Meshes.at(index); }
	private:
        void ProcessNode(FbxNode* node);
        void ProcessMesh(FbxNode* node);
        void ReadVertex(FbxMesh* mesh, int index, Vertex& vertex);
		void ReadNormal(FbxMesh* mesh, int index, int vertexCounter, Vertex& vertex);
    private:
		std::filesystem::path m_FilePath;
        std::vector<Mesh> m_Meshes;
		FbxNode* m_RootNode = nullptr;
	private:
        FbxImporter* m_Importer = nullptr;
	};
}
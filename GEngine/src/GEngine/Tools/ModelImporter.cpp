#include "GEpch.h"
#include "ModelImporter.h"

namespace GEngine
{
    ModelImporter::~ModelImporter()
    {
	    
    }
    bool ModelImporter::LoadMesh(const std::string filaName)
	{
        m_FilePath = filaName;
        FbxManager* manager = FbxManager::Create();
        FbxIOSettings* settings = FbxIOSettings::Create(manager, IOSROOT);
        manager->SetIOSettings(settings); 
        m_Importer = FbxImporter::Create(manager, "");
        if (!m_Importer->Initialize(filaName.c_str(), -1))
		{
			GE_CORE_ERROR("Failed to load model: {0}", filaName);
			return false;
		}
        FbxScene* scene = FbxScene::Create(manager, "");
        m_Importer->Import(scene);
        FbxGeometryConverter geomConverter(manager);
        geomConverter.Triangulate(scene, true);
        FbxNode* m_RootNode = scene->GetRootNode();
        if (m_RootNode)
        {
            GE_TRACE("Root Node: {0}", m_RootNode->GetName());

            int childCount = m_RootNode->GetChildCount();
            for (int i = 0; i < childCount; i++)
                ProcessNode(m_RootNode->GetChild(i));
        }
        m_Importer->Destroy();
        m_Meshes.clear();
		return true;
	}



    void ModelImporter::ProcessNode(FbxNode* node)
    {

        if (node->GetNodeAttribute())
        {
            switch (node->GetNodeAttribute()->GetAttributeType())
            {
            case FbxNodeAttribute::eMesh:
                ProcessMesh(node);
                break;
           /* case FbxNodeAttribute::eSkeleton:
                ProcessSkeleton(node);
                break;
            case FbxNodeAttribute::eLight:
                ProcessLight(node);
                break;
            case FbxNodeAttribute::eCamera:
                ProcessCamera();
                break;*/
            }
        }

        for (int i = 0; i < node->GetChildCount(); ++i)
        {
            ProcessNode(node->GetChild(i));
        }

        for (int i = 0; i < m_Meshes.size(); i++)
        {
            MeshLibrary::AddMesh_Force(m_Meshes[i]);
        }
    }
    void ModelImporter::ProcessMesh(FbxNode* node)
    {
        FbxMesh* mesh = node->GetMesh();

        Mesh newMesh;
        newMesh.m_Name = mesh->GetName();
        newMesh.m_SourceFilePath = m_FilePath;
        int triangleCount = mesh->GetPolygonCount();
        int vertexCounter = 0;
        GE_TRACE("Mesh Name: {0}", newMesh.m_Name);
        GE_TRACE("Triangle Count: {0}", triangleCount);
        GE_TRACE("Vertex Count: {0}", mesh->GetControlPointsCount());
        Vertex vertex;
        for (int i = 0; i < triangleCount; i++)
        {
            Triangle triangle;
            for (int j = 0; j < 3; j++)
            {
                int ctrlPointIndex = mesh->GetPolygonVertex(i, j);

                ReadVertex(mesh, ctrlPointIndex, vertex);
                ReadNormal(mesh, ctrlPointIndex, vertexCounter, vertex);

                newMesh.m_Vertices.push_back(vertex);
                triangle.Vertices[j] = vertex;
                vertexCounter++;
            }
            newMesh.m_Triangles.push_back(triangle);
        }
        for (long long i = 0; i < newMesh.m_Vertices.size(); i++)
        {
            newMesh.m_Indices.push_back(i);
        }
		m_Meshes.push_back(newMesh);
    }
    void ModelImporter::ReadVertex(FbxMesh* mesh, int index, Vertex& vertex)
    {
        FbxVector4* pCtrlPoint = mesh->GetControlPoints();

        vertex.m_Position.x = pCtrlPoint[index].mData[0];
        vertex.m_Position.y = pCtrlPoint[index].mData[1];
        vertex.m_Position.z = pCtrlPoint[index].mData[2];
        vertex.m_Position.w = 1.0f;
    }
    void ModelImporter::ReadNormal(FbxMesh* mesh, int index, int vertexCounter, Vertex& vertex)
    {
        if (mesh->GetElementNormalCount() < 1)
        {
            return;
        }
        FbxGeometryElementNormal* normal = mesh->GetElementNormal(0);
        switch (normal->GetMappingMode())
        {
        case FbxGeometryElement::eByControlPoint:
        {
            switch (normal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                vertex.m_Normal.x = normal->GetDirectArray().GetAt(index).mData[0];
                vertex.m_Normal.y = normal->GetDirectArray().GetAt(index).mData[1];
                vertex.m_Normal.z = normal->GetDirectArray().GetAt(index).mData[2];
                vertex.m_Normal.w = 0.0f;
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = normal->GetIndexArray().GetAt(index);
                vertex.m_Normal.x = normal->GetDirectArray().GetAt(id).mData[0];
                vertex.m_Normal.y = normal->GetDirectArray().GetAt(id).mData[1];
                vertex.m_Normal.z = normal->GetDirectArray().GetAt(id).mData[2];
                vertex.m_Normal.w = 0.0f;
            }
            break;

            default:
                break;
            }
        }
        break;

        case FbxGeometryElement::eByPolygonVertex:
        {
            switch (normal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                vertex.m_Normal.x = normal->GetDirectArray().GetAt(vertexCounter).mData[0];
                vertex.m_Normal.y = normal->GetDirectArray().GetAt(vertexCounter).mData[1];
                vertex.m_Normal.z = normal->GetDirectArray().GetAt(vertexCounter).mData[2];
                vertex.m_Normal.w = 0.0f;
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = normal->GetIndexArray().GetAt(vertexCounter);
                vertex.m_Normal.x = normal->GetDirectArray().GetAt(id).mData[0];
                vertex.m_Normal.y = normal->GetDirectArray().GetAt(id).mData[1];
                vertex.m_Normal.z = normal->GetDirectArray().GetAt(id).mData[2];
                vertex.m_Normal.w = 0.0f;
            }
            break;

            default:
                break;
            }
        }
        break;
        }
    }
}

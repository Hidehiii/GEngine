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
        GE_CORE_TRACE("Mesh Name: {0}", newMesh.m_Name);
        GE_CORE_TRACE("Triangle Count: {0}", triangleCount);
        GE_CORE_TRACE("Vertex Count: {0}", mesh->GetControlPointsCount());
		GE_CORE_INFO("UV layers Count: {}", mesh->GetElementUVCount());
        Vertex vertex;
        for (int i = 0; i < triangleCount; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                int ctrlPointIndex = mesh->GetPolygonVertex(i, j);

                ReadVertex(mesh, ctrlPointIndex, vertex);
                ReadColor(mesh, ctrlPointIndex, vertexCounter, vertex);
                ReadNormal(mesh, ctrlPointIndex, vertexCounter, vertex);
                ReadTangent(mesh, ctrlPointIndex, vertexCounter, vertex);
				ReadUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), 0, &vertex.m_UV0);
				ReadUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), 1, &vertex.m_UV1);
				ReadUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), 2, &vertex.m_UV2);
				ReadUV(mesh, ctrlPointIndex, mesh->GetTextureUVIndex(i, j), 3, &vertex.m_UV3);

                if (newMesh.m_VertexsIndex.count(vertex) == 0)
                {
                    newMesh.m_VertexsIndex[vertex] = static_cast<uint32_t>(newMesh.m_Vertices.size());
                    newMesh.m_Vertices.push_back(vertex);
                }
                newMesh.m_Indices.push_back(newMesh.m_VertexsIndex[vertex]);
                vertexCounter++;
            }
        }
		m_Meshes.push_back(newMesh);
    }
    void ModelImporter::ReadVertex(FbxMesh* mesh, int ctrlPointIndex, Vertex& vertex)
    {
        FbxVector4* pCtrlPoint = mesh->GetControlPoints();

        vertex.m_Position.x = pCtrlPoint[ctrlPointIndex].mData[0];
        vertex.m_Position.y = pCtrlPoint[ctrlPointIndex].mData[1];
        vertex.m_Position.z = pCtrlPoint[ctrlPointIndex].mData[2];
        vertex.m_Position.w = 1.0f;
    }
    void ModelImporter::ReadNormal(FbxMesh* mesh, int ctrlPointIndex, int vertexCounter, Vertex& vertex)
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
                vertex.m_Normal.x = normal->GetDirectArray().GetAt(ctrlPointIndex).mData[0];
                vertex.m_Normal.y = normal->GetDirectArray().GetAt(ctrlPointIndex).mData[1];
                vertex.m_Normal.z = normal->GetDirectArray().GetAt(ctrlPointIndex).mData[2];
                vertex.m_Normal.w = 0.0f;
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int id = normal->GetIndexArray().GetAt(ctrlPointIndex);
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
    void ModelImporter::ReadColor(FbxMesh* mesh, int ctrlPointIndex, int vertexCounter, Vertex& vertex)
    {
		if (mesh->GetElementVertexColorCount() < 1)
		{
			return;
		}
		FbxGeometryElementVertexColor* pVertexColor = mesh->GetElementVertexColor(0);
		switch (pVertexColor->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (pVertexColor->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				vertex.m_Color.x = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mRed;
				vertex.m_Color.y = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mGreen;
				vertex.m_Color.z = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mBlue;
				vertex.m_Color.w = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mAlpha;
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = pVertexColor->GetIndexArray().GetAt(ctrlPointIndex);
				vertex.m_Color.x = pVertexColor->GetDirectArray().GetAt(id).mRed;
				vertex.m_Color.y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
				vertex.m_Color.z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
				vertex.m_Color.w = pVertexColor->GetDirectArray().GetAt(id).mAlpha;
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (pVertexColor->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				vertex.m_Color.x = pVertexColor->GetDirectArray().GetAt(vertexCounter).mRed;
				vertex.m_Color.y = pVertexColor->GetDirectArray().GetAt(vertexCounter).mGreen;
				vertex.m_Color.z = pVertexColor->GetDirectArray().GetAt(vertexCounter).mBlue;
				vertex.m_Color.w = pVertexColor->GetDirectArray().GetAt(vertexCounter).mAlpha;
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				int id = pVertexColor->GetIndexArray().GetAt(vertexCounter);
				vertex.m_Color.x = pVertexColor->GetDirectArray().GetAt(id).mRed;
				vertex.m_Color.y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
				vertex.m_Color.z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
				vertex.m_Color.w = pVertexColor->GetDirectArray().GetAt(id).mAlpha;
			}
			break;
			default:
				break;
			}
		}
		break;
		}
    }
    void ModelImporter::ReadTangent(FbxMesh* mesh, int ctrlPointIndex, int vertexCounter, Vertex& vertex)
    {
		if (mesh->GetElementTangentCount() < 1)
		{
			return;
		}
		FbxGeometryElementTangent* leTangent = mesh->GetElementTangent(0);

		switch (leTangent->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (leTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				vertex.m_Tangent.x = leTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[0];
				vertex.m_Tangent.y = leTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[1];
				vertex.m_Tangent.z = leTangent->GetDirectArray().GetAt(ctrlPointIndex).mData[2];
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = leTangent->GetIndexArray().GetAt(ctrlPointIndex);
				vertex.m_Tangent.x = leTangent->GetDirectArray().GetAt(id).mData[0];
				vertex.m_Tangent.y = leTangent->GetDirectArray().GetAt(id).mData[1];
				vertex.m_Tangent.z = leTangent->GetDirectArray().GetAt(id).mData[2];
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (leTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				vertex.m_Tangent.x = leTangent->GetDirectArray().GetAt(vertexCounter).mData[0];
				vertex.m_Tangent.y = leTangent->GetDirectArray().GetAt(vertexCounter).mData[1];
				vertex.m_Tangent.z = leTangent->GetDirectArray().GetAt(vertexCounter).mData[2];
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = leTangent->GetIndexArray().GetAt(vertexCounter);
				vertex.m_Tangent.x = leTangent->GetDirectArray().GetAt(id).mData[0];
				vertex.m_Tangent.y = leTangent->GetDirectArray().GetAt(id).mData[1];
				vertex.m_Tangent.z = leTangent->GetDirectArray().GetAt(id).mData[2];
			}
			break;

			default:
				break;
			}
		}
		break;
		}
    }
    void ModelImporter::ReadUV(FbxMesh* mesh, int ctrlPointIndex, int textureUVIndex, int uvLayer, Vector2* pUV)
    {
		if (uvLayer >= 4 || mesh->GetElementUVCount() <= uvLayer)
		{
			return;
		}
		FbxGeometryElementUV* pVertexUV = mesh->GetElementUV(uvLayer);

		switch (pVertexUV->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (pVertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				pUV->x = pVertexUV->GetDirectArray().GetAt(ctrlPointIndex).mData[0];
				pUV->y = pVertexUV->GetDirectArray().GetAt(ctrlPointIndex).mData[1];
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);
				pUV->x = pVertexUV->GetDirectArray().GetAt(id).mData[0];
				pUV->y = pVertexUV->GetDirectArray().GetAt(id).mData[1];
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (pVertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			case FbxGeometryElement::eIndexToDirect:
			{
				pUV->x = pVertexUV->GetDirectArray().GetAt(textureUVIndex).mData[0];
				pUV->y = pVertexUV->GetDirectArray().GetAt(textureUVIndex).mData[1];
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

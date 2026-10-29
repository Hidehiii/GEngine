#include "GEpch.h"
#include "GEngine/Tools/ModelImporter.h"
#include "GEngine/Asset/GAsset.h"
#include "GEngine/Runtime/ResourcePack.h"
#include <stdexcept>

namespace GEngine
{
    namespace
    {
        void DestroyFbxObjects(FbxImporter* importer, FbxScene* scene,
            FbxIOSettings* settings, FbxManager* manager)
        {
            if (importer) importer->Destroy();
            if (scene) scene->Destroy();
            if (settings) settings->Destroy();
            if (manager) manager->Destroy();
        }

        class ResourceFbxStream final : public FbxStream
        {
        public:
            explicit ResourceFbxStream(std::vector<char> bytes) : Bytes(std::move(bytes)) {}
            EState GetState() override { return State; }
            bool Open(void*) override { Position = 0; Error = 0; State = eOpen; return true; }
            bool Close() override { Position = 0; State = eClosed; return true; }
            bool Flush() override { return false; }
            size_t Write(const void*, FbxUInt64) override { Error = 1; return 0; }
            size_t Read(void* target, FbxUInt64 size) const override
            {
                if (State != eOpen) { Error = 1; return 0; }
                const auto count = static_cast<size_t>((std::min)(size, static_cast<FbxUInt64>(Bytes.size() - Position)));
                if (count) memcpy(target, Bytes.data() + Position, count);
                Position += count; return count;
            }
            int GetReaderID() const override { return -1; }
            int GetWriterID() const override { return -1; }
            void Seek(const FbxInt64& offset, const FbxFile::ESeekPos& origin) override
            {
                const FbxInt64 base = origin == FbxFile::eBegin ? 0 :
                    origin == FbxFile::eEnd ? static_cast<FbxInt64>(Bytes.size()) : static_cast<FbxInt64>(Position);
                if (offset < -base || offset > static_cast<FbxInt64>(Bytes.size()) - base) { Error = 1; return; }
                SetPosition(base + offset);
            }
            FbxInt64 GetPosition() const override { return static_cast<FbxInt64>(Position); }
            void SetPosition(FbxInt64 position) override
            {
                if (position < 0 || static_cast<FbxUInt64>(position) > Bytes.size()) Error = 1;
                else
                {
                    Position = static_cast<size_t>(position);
                    Error = 0;
                }
            }
            int GetError() const override { return Error; }
            void ClearError() override { Error = 0; }
        private:
            std::vector<char> Bytes;
            mutable size_t Position = 0;
            mutable int Error = 0;
            EState State = eClosed;
        };
    }
    ModelImporter::~ModelImporter()
    {
	    
    }
    bool ModelImporter::LoadMesh(const std::string filaName)
	{
        m_FilePath = filaName;
        FbxManager* manager = FbxManager::Create();
        if (!manager) throw std::runtime_error("Cannot create FBX manager.");
        const auto destroyManager = [](FbxManager* value) { value->Destroy(); };
        std::unique_ptr<FbxManager, decltype(destroyManager)> managerOwner(manager, destroyManager);
        FbxIOSettings* settings = FbxIOSettings::Create(manager, IOSROOT);
        if (!settings) throw std::runtime_error("Cannot create FBX IO settings.");
        manager->SetIOSettings(settings);
        FbxImporter* importer = FbxImporter::Create(manager, "");
        if (!importer) throw std::runtime_error("Cannot create FBX importer.");
        std::unique_ptr<ResourceFbxStream> stream;
        const auto path = std::filesystem::u8path(filaName);
        if (path.extension() == ".gasset")
        {
            const auto envelope = LoadGAsset(path, "mesh");
            std::string format = envelope.Format;
            std::transform(format.begin(), format.end(), format.begin(), ::tolower);
            if (format != "fbx") throw std::runtime_error("GAsset model import currently supports FBX payloads only.");
            stream = std::make_unique<ResourceFbxStream>(envelope.Payload);
        }
        else if (HasResourcePack())
        {
            auto extension = path.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (extension != ".fbx") throw std::runtime_error("Archive model import currently supports FBX streams only.");
            stream = std::make_unique<ResourceFbxStream>(ReadResource(path));
        }
        bool initialized = false;
        const auto readerId = manager->GetIOPluginRegistry()->FindReaderIDByExtension("fbx");
        if (stream)
        {
            initialized = importer->Initialize(stream.get(), nullptr, readerId, settings);
        }
        else initialized = importer->Initialize(filaName.c_str(), -1);
        FbxScene* scene = nullptr;
        if (!initialized)
		{
            GE_CORE_ERROR("Failed to load model: {0}: {1}", filaName, importer->GetStatus().GetErrorString());
            DestroyFbxObjects(importer, scene, settings, manager);
			return false;
		}
        scene = FbxScene::Create(manager, "");
        if (!scene) throw std::runtime_error("Cannot create FBX scene.");
        if (!importer->Import(scene))
        {
            DestroyFbxObjects(importer, scene, settings, manager);
            return false;
        }
        FbxGeometryConverter geomConverter(manager);
        geomConverter.Triangulate(scene, true);
        FbxNode* rootNode = scene->GetRootNode();
        if (rootNode)
        {
            GE_TRACE("Root Node: {0}", rootNode->GetName());

            int childCount = rootNode->GetChildCount();
            for (int i = 0; i < childCount; i++)
                ProcessNode(rootNode->GetChild(i));
        }
        DestroyFbxObjects(importer, scene, settings, manager);
        managerOwner.release();
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

        /*for (int i = 0; i < m_Meshes.size(); i++)
        {
            MeshLibrary::AddMesh_Force(m_Meshes[i]);
        }*/
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
        GE_CORE_INFO("GAsset mesh processed: {} vertices, {} indices",
            newMesh.m_Vertices.size(), newMesh.m_Indices.size());
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

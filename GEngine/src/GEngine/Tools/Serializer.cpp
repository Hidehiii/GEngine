#include "GEpch.h"
#include "GEngine/Tools/Serializer.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Components/Components.h"
#include <yaml-cpp/yaml.h>
#include "GEngine/Object/GameObject.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/Material.h"
#include "GEngine/Core/Config.h"
#include<filesystem>
#include <cmath>
#include "GEngine/Tools/FileSystemHelper.h"
#include "GEngine/Runtime/ResourcePack.h"

namespace YAML
{
	template <>
	struct convert<GEngine::Vector2>
	{
		static Node encode(const GEngine::Vector2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, GEngine::Vector2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template <>
	struct convert<GEngine::Vector3>
	{
		static Node encode(const GEngine::Vector3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, GEngine::Vector3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template <>
	struct convert<GEngine::Vector4>
	{
		static Node encode(const GEngine::Vector4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, GEngine::Vector4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template <>
	struct convert<GEngine::Quaternion>
	{
		static Node encode(const GEngine::Quaternion& rhs)
		{
			Node node;
			
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, GEngine::Quaternion& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace GEngine
{
	YAML::Emitter& operator<< (YAML::Emitter& out, const Vector2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<< (YAML::Emitter& out, const Vector3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<< (YAML::Emitter& out, const Vector4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z <<v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<< (YAML::Emitter& out, const Quaternion& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
    static void SerializeComponentMaterial(YAML::Emitter& out, const Ref<Material>& material)
    {
        out << YAML::Key << "Material" << YAML::Value;
        if (!material) { out << YAML::Null; return; }
        const auto path = material->GetShader()->GetShaderPath();
        if (std::filesystem::u8path(path).extension() != ".gasset")
            throw std::runtime_error("Scene materials require a persistent shader .gasset.");
        out << YAML::BeginMap << YAML::Key << "Shader" << YAML::Value << path;
        out << YAML::Key << "Name" << YAML::Value << material->GetName() << YAML::EndMap;
    }
    static void SerializeGameObject(YAML::Emitter& out, GameObject object)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "GameObject" << YAML::Value << static_cast<uint64_t>(object.GetUUID());
        out << YAML::Key << "Name" << YAML::Value << object.GetName();
        out << YAML::Key << "Active" << YAML::Value << object.IsActive();
        if (auto parent = object.Parent())
            out << YAML::Key << "Parent" << YAML::Value << static_cast<uint64_t>(parent.GetUUID());
        auto& transform = object.GetTransform();
        out << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "Position" << YAML::Value << transform.GetPosition();
        out << YAML::Key << "Rotation" << YAML::Value << transform.GetRotation();
        out << YAML::Key << "Scale" << YAML::Value << transform.GetScale() << YAML::EndMap;
        if (!object.GetScriptClasses().empty())
        {
            out << YAML::Key << "Scripts" << YAML::Value << YAML::BeginSeq;
            for (const auto& script : object.GetScriptClasses()) out << script;
            out << YAML::EndSeq;
        }
        if (auto* camera = object.TryGetComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "CameraParameters" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "CameraType" << YAML::Value << static_cast<int>(camera->GetCameraType());
            out << YAML::Key << "OrthoGraphicSize" << YAML::Value << camera->m_OrthoGraphicSize;
            out << YAML::Key << "OrthoGraphicNear" << YAML::Value << camera->m_OrthoGraphicNear;
            out << YAML::Key << "OrthoGraphicFar" << YAML::Value << camera->m_OrthoGraphicFar;
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera->m_PerspectiveFOV;
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera->m_PerspectiveNear;
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera->m_PerspectiveFar << YAML::EndMap;
            out << YAML::Key << "RenderOrder" << YAML::Value << camera->RenderOrder;
            out << YAML::Key << "isPrimary" << YAML::Value << camera->isPrimary << YAML::EndMap;
        }
        if (auto* image = object.TryGetComponent<ImageComponent>())
        {
            out << YAML::Key << "ImageComponent" << YAML::Value << YAML::BeginMap;
            const auto asset = image->GetImageAsset();
            if (asset && std::filesystem::u8path(asset->GetPath()).extension() != ".gasset")
                throw std::runtime_error("Cannot serialize an ImageComponent without a persistent image .gasset path.");
            out << YAML::Key << "ImageAsset" << YAML::Value << (asset ? asset->GetPath() : "");
            out << YAML::Key << "Color" << YAML::Value << image->Color;
            out << YAML::Key << "Tiling" << YAML::Value << image->Tiling;
            SerializeComponentMaterial(out, image->GetMaterial());
            out << YAML::EndMap;
        }
        if (auto* mesh = object.TryGetComponent<StaticMeshComponent>())
        {
            if (mesh->GetMeshAsset() && std::filesystem::u8path(mesh->GetMeshAssetPath()).extension() != ".gasset")
                throw std::runtime_error("Cannot serialize a procedural mesh without a persistent mesh .gasset path.");
            out << YAML::Key << "StaticMeshComponent" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "MeshAsset" << YAML::Value << mesh->GetMeshAssetPath();
            out << YAML::Key << "MeshIndex" << YAML::Value << mesh->GetMeshIndex();
            SerializeComponentMaterial(out, mesh->GetMaterial());
            out << YAML::EndMap;
        }
        if (auto* light = object.TryGetComponent<DirectionalLightComponent>())
        {
            out << YAML::Key << "DirectionalLightComponent" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Color" << YAML::Value << light->Color;
            out << YAML::Key << "Intensity" << YAML::Value << light->Intensity << YAML::EndMap;
        }
        out << YAML::EndMap;
    }

	template<typename T>
	static void Serializer::Serialize(const std::string& filepath, Ref<T>& data)
	{
		static_assert(false);
	}

	template<typename T>
	void Serializer::Serialize(const std::string& filepath, T& data)
	{
		static_assert(false);
	}

	// 编码config
	template <>
	static void GENGINE_API Serializer::Serialize(const std::string& filepath, Ref<Config>& config)
	{
		EnsureResourceWritable(std::filesystem::u8path(filepath));
		if (!std::filesystem::exists(filepath))
		{
			GE_CORE_INFO("Creating config file...");
			std::filesystem::path	path = std::filesystem::current_path() / filepath;
			std::fstream			file(path, std::ios::out | std::ios::trunc);
		}
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "GraphicsAPI" << YAML::Value << (int)config->m_GraphicsAPI;
		out << YAML::Key << "FramesInFlight" << YAML::Value << (int)config->m_FramesInFlight;
		out << YAML::Key << "VSync" << YAML::Value << (int)config->m_VSync;
		out << YAML::Key << "CommandBufferCount" << YAML::Value << (int)config->m_CommandBufferCount;
		out << YAML::Key << "ShaderCacheDirectory" << YAML::Value << config->m_ShaderCacheDirectory;
		out << YAML::Key << "LongPressThresholdMs" << YAML::Value << (int)config->m_LongPressThresholdMs;
		out << YAML::Key << "DoubleClickThresholdMs" << YAML::Value << (int)config->m_DoubleClickThresholdMs;
		out << YAML::Key << "WindowManagerAPI" << YAML::Value << (int)config->m_WindowManagerAPI;

		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	// 编码场景的YAML数据
	template <>
	static void GENGINE_API Serializer::Serialize(const std::string& filepath, Ref<Scene>& scene)
	{
		EnsureResourceWritable(std::filesystem::u8path(filepath));
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << scene->name;
		out << YAML::Key << "GameObjects";
		out << YAML::Value << YAML::BeginSeq;
		auto view = scene->m_Registry.view<GameObjectData>();
		for (auto entity : view)
		{
			GameObject gameObject = { entity, scene.get()};
			if (!gameObject)
				continue;

			SerializeGameObject(out, gameObject);
			
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	// 编码材质的YAML数据
	template <>
	static void GENGINE_API Serializer::Serialize(const std::string& filepath, Ref<Material>& material)
	{
		EnsureResourceWritable(std::filesystem::u8path(filepath));
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material";
		out << YAML::Value << material->GetName();
		out << YAML::Key << "Shader";
		out << YAML::Value << YAML::BeginSeq;

		out << YAML::Key << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << material->GetShader()->GetShaderName();
		out << YAML::Key << "Properties";
		out << YAML::Value << YAML::BeginMap;
		/*std::vector<ShaderUniform>& uniforms = material->GetUniforms();
		for (auto uniform : uniforms)
		{
			out << YAML::Key << uniform.Name;
			out << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Type" << YAML::Value << (int)uniform.Type;
			switch (uniform.Type)
			{
			case ShaderUniformType::Int:		out << YAML::Key << "Value" << YAML::Value << material->GetInt(uniform.Name); break;
			case ShaderUniformType::Float:		out << YAML::Key << "Value" << YAML::Value << material->GetFloat(uniform.Name); break;
			case ShaderUniformType::Vector:
			case ShaderUniformType::Color:		out << YAML::Key << "Value" << YAML::Value << material->GetVector(uniform.Name); break;
			default: GE_CORE_CRITICAL("There is an unknown uniform type try to be writen into YAML file.");
			}
			out << YAML::EndMap;
		}*/
		out << YAML::EndMap;
		out << YAML::EndMap;

		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	// 编码shader cache数据
	template <>
	static void GENGINE_API Serializer::Serialize(const std::string& filepath, ShaderCacheInfo& cache)
	{
		EnsureResourceWritable(std::filesystem::u8path(filepath));
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << cache.Name;
		out << YAML::Key << "SpirvHash" << YAML::Value << cache.SpirvHash;
		out << YAML::Key << "DxilHash" << YAML::Value << cache.DxilHash;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}


	template<typename T>
	static void Serializer::Deserialize(const std::string& filepath, Ref<T>& data)
	{
		static_assert(false);
	}

	template<typename T>
	void Serializer::Deserialize(const std::string& filepath, T& data)
	{
		static_assert(false);
	}

	// 解码config数据
	template <>
	static void  GENGINE_API Serializer::Deserialize(const std::string& filepath, Ref<Config>& config)
	{
		if(FileSystemHelper::IsDocument(filepath) == false)
		{
			GE_CORE_INFO("Failed to load config file: {0}, because it is not a document.", filepath);
			return;
		}
		YAML::Node data;
		if (!std::filesystem::exists(filepath))
		{
			GE_CORE_INFO("Missins config file...");
			return;
		}
		try
		{
			data = YAML::Load(FileSystemHelper::ReadFileAsString(filepath));
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load config file: {0}, \n{1}", filepath, e.what());
			return;
		}

		config->m_GraphicsAPI							= data["GraphicsAPI"].as<uint8_t>();
		config->m_FramesInFlight						= data["FramesInFlight"].as<uint8_t>();
		config->m_VSync									= data["VSync"].as<int>();
		config->m_CommandBufferCount					= data["CommandBufferCount"].as<uint32_t>();
		config->m_ShaderCacheDirectory = HasResourcePack()
            ? "Cache/Shaders/" + std::to_string(config->m_GraphicsAPI) + "/"
            : data["ShaderCacheDirectory"].as<std::string>();
		config->m_LongPressThresholdMs					= data["LongPressThresholdMs"].as<uint32_t>();
		config->m_DoubleClickThresholdMs				= data["DoubleClickThresholdMs"].as<uint32_t>();
		config->m_WindowManagerAPI						= data["WindowManagerAPI"].as<uint8_t>();
	}
	
	// 解码场景的YAML数据
	template <>
    static void GENGINE_API Serializer::Deserialize(const std::string& filepath, Ref<Scene>& scene)
    {
        const auto data = YAML::Load(FileSystemHelper::ReadFileAsString(filepath));
        if (!data.IsMap() || !data["Scene"] || !data["GameObjects"] || !data["GameObjects"].IsSequence())
            throw std::runtime_error("Invalid scene document: " + filepath);
        auto candidate = CreateRef<Scene>();
        candidate->name = data["Scene"].as<std::string>();
        std::unordered_map<uint64_t, GameObject> objects;
        auto loadMaterial = [](const YAML::Node& node) -> Ref<Material>
        {
            if (!node || node.IsNull()) return {};
            const auto shader = node["Shader"].as<std::string>();
            if (std::filesystem::u8path(shader).extension() != ".gasset")
                throw std::runtime_error("Scene material shader must use .gasset.");
            return Material::Create(Shader::Create(shader), node["Name"].as<std::string>());
        };
        for (const auto& node : data["GameObjects"])
        {
            if (!node.IsMap()) throw std::runtime_error("GameObject must be a map.");
            for (const auto& entry : node)
            {
                const auto key = entry.first.as<std::string>();
                if (key != "GameObject" && key != "Name" && key != "Active" && key != "Attribute" &&
                    key != "Transform" && key != "Parent" && key != "Scripts" &&
                    key != "CameraComponent" && key != "ImageComponent" &&
                    key != "DirectionalLightComponent" && key != "StaticMeshComponent")
                    throw std::runtime_error("Removed or unknown scene component: " + key);
            }
            const auto id = node["GameObject"].as<uint64_t>();
            auto object = candidate->CreateGameObject(id, node["Name"] ? node["Name"].as<std::string>() : "");
            objects.emplace(id, object);
            if (node["Active"]) object.SetActive(node["Active"].as<bool>());
            else if (node["Attribute"]) object.SetActive(node["Attribute"]["IsActive"].as<bool>());
            if (const auto transform = node["Transform"])
            {
                object.GetTransform().SetPosition(transform["Position"].as<Vector3>());
                object.GetTransform().SetRotation(transform["Rotation"].as<Quaternion>());
                object.GetTransform().SetScale(transform["Scale"].as<Vector3>());
            }
            if (const auto scripts = node["Scripts"])
            {
                if (!scripts.IsSequence()) throw std::runtime_error("GameObject Scripts must be a sequence.");
                for (const auto& script : scripts)
                {
                    if (!script.IsScalar()) throw std::runtime_error("GameObject script class must be a scalar.");
                    object.AddScript(script.as<std::string>());
                }
            }
            if (const auto nodeCamera = node["CameraComponent"])
            {
                const auto parameters = nodeCamera["CameraParameters"];
                const auto type = parameters["CameraType"].as<int>();
                if (type < 0 || type > 1) throw std::runtime_error("Invalid CameraType.");
                object.AddComponent<CameraComponent>(static_cast<CameraType>(type));
                auto& camera = object.GetComponent<CameraComponent>();
                camera.m_OrthoGraphicSize = parameters["OrthoGraphicSize"].as<float>();
                camera.m_OrthoGraphicNear = parameters["OrthoGraphicNear"].as<float>();
                camera.m_OrthoGraphicFar = parameters["OrthoGraphicFar"].as<float>();
                camera.m_PerspectiveFOV = parameters["PerspectiveFOV"].as<float>();
                camera.m_PerspectiveNear = parameters["PerspectiveNear"].as<float>();
                camera.m_PerspectiveFar = parameters["PerspectiveFar"].as<float>();
                camera.RenderOrder = nodeCamera["RenderOrder"].as<int>();
                camera.isPrimary = nodeCamera["isPrimary"].as<bool>();
            }
            if (const auto nodeImage = node["ImageComponent"])
            {
                object.AddComponent<ImageComponent>();
                auto& image = object.GetComponent<ImageComponent>();
                const auto path = nodeImage["ImageAsset"].as<std::string>();
                if (!path.empty()) image.LoadImageAsset(path);
                image.Color = nodeImage["Color"].as<Vector4>();
                image.Tiling = nodeImage["Tiling"].as<Vector2>();
                if (auto material = loadMaterial(nodeImage["Material"])) image.SetMaterial(material);
            }
            if (const auto nodeMesh = node["StaticMeshComponent"])
            {
                object.AddComponent<StaticMeshComponent>();
                auto& mesh = object.GetComponent<StaticMeshComponent>();
                const auto path = nodeMesh["MeshAsset"].as<std::string>();
                if (!path.empty()) mesh.LoadMeshAsset(path, nodeMesh["MeshIndex"].as<uint32_t>());
                if (auto material = loadMaterial(nodeMesh["Material"])) mesh.SetMaterial(material);
            }
            if (const auto nodeLight = node["DirectionalLightComponent"])
            {
                object.AddComponent<DirectionalLightComponent>();
                auto& light = object.GetComponent<DirectionalLightComponent>();
                light.Color = nodeLight["Color"].as<Vector4>();
                light.Intensity = nodeLight["Intensity"].as<float>();
                if (!std::isfinite(light.Intensity) || light.Intensity < 0.0f)
                    throw std::runtime_error("DirectionalLightComponent intensity must be finite and nonnegative.");
            }
        }
        for (const auto& node : data["GameObjects"])
            if (node["Parent"])
            {
                const auto parent = node["Parent"].as<uint64_t>();
                if (!objects.count(parent)) throw std::runtime_error("Scene parent reference does not exist.");
                objects.at(node["GameObject"].as<uint64_t>()).SetParent(objects.at(parent));
            }
        scene = std::move(candidate);
    }

	// 解码材质的YAML数据
	template <>
	static void GENGINE_API Serializer::Deserialize(const std::string& filepath, Ref<Material>& material)
	{
		if(FileSystemHelper::IsDocument(filepath) == false)
		{
			GE_CORE_INFO("Material file is missing: {0}", filepath);
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::Load(FileSystemHelper::ReadFileAsString(filepath));
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load material file: {0}, \n{1}", filepath, e.what());
			return;
		}
		if (!data["Material"])
			return;

		std::string materialName = data["Material"].as<std::string>();
		material->SetName(materialName);

		auto shader = data["Shader"];
		if (shader)
		{
			// TODO 
			GE_CORE_CRITICAL("这里的材质解码还没写");
		}
	}

	// 解码shader cache数据
	template <>
	static void GENGINE_API Serializer::Deserialize(const std::string& filepath, ShaderCacheInfo& cache)
	{
		if(FileSystemHelper::IsDocument(filepath) == false)
		{
			GE_CORE_INFO("Shader cache file is missing: {0}", filepath);
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::Load(FileSystemHelper::ReadFileAsString(filepath));
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load shader cache file: {0}, \n{1}", filepath, e.what());
			return;
		}

		cache.Name		= data["Name"].as<std::string>();
		cache.SpirvHash	= data["SpirvHash"].as<std::string>();
		cache.DxilHash	= data["DxilHash"].as<std::string>();
	}
}

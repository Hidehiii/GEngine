#include "GEpch.h"
#include "Serializer.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Components/Components.h"
#include <yaml-cpp/yaml.h>
#include "GEngine/Object/GameObject.h"
#include "GEngine/Math/Math.h"

namespace YAML
{
	template <>
	struct convert<GEngine::Vector2>
	{
		static Node encode(const GEngine::Vector2& rhs)
		{
			Node node;
			node.push_back(rhs.value.x);
			node.push_back(rhs.value.y);
			return node;
		}

		static bool decode(const Node& node, GEngine::Vector2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.value.x = node[0].as<float>();
			rhs.value.y = node[1].as<float>();
			return true;
		}
	};

	template <>
	struct convert<GEngine::Vector3>
	{
		static Node encode(const GEngine::Vector3& rhs)
		{
			Node node;
			node.push_back(rhs.value.x);
			node.push_back(rhs.value.y);
			node.push_back(rhs.value.z);
			return node;
		}

		static bool decode(const Node& node, GEngine::Vector3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.value.x = node[0].as<float>();
			rhs.value.y = node[1].as<float>();
			rhs.value.z = node[2].as<float>();
			return true;
		}
	};

	template <>
	struct convert<GEngine::Vector4>
	{
		static Node encode(const GEngine::Vector4& rhs)
		{
			Node node;
			node.push_back(rhs.value.x);
			node.push_back(rhs.value.y);
			node.push_back(rhs.value.z);
			node.push_back(rhs.value.w);
			return node;
		}

		static bool decode(const Node& node, GEngine::Vector4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.value.x = node[0].as<float>();
			rhs.value.y = node[1].as<float>();
			rhs.value.z = node[2].as<float>();
			rhs.value.w = node[3].as<float>();
			return true;
		}
	};

	template <>
	struct convert<GEngine::Quaternion>
	{
		static Node encode(const GEngine::Quaternion& rhs)
		{
			Node node;
			
			node.push_back(rhs.value.x);
			node.push_back(rhs.value.y);
			node.push_back(rhs.value.z);
			node.push_back(rhs.value.w);
			return node;
		}

		static bool decode(const Node& node, GEngine::Quaternion& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.value.x = node[0].as<float>();
			rhs.value.y = node[1].as<float>();
			rhs.value.z = node[2].as<float>();
			rhs.value.w = node[3].as<float>();
			return true;
		}
	};
}

namespace GEngine
{
	YAML::Emitter& operator<< (YAML::Emitter& out, const Vector2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.value.x << v.value.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<< (YAML::Emitter& out, const Vector3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.value.x << v.value.y << v.value.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<< (YAML::Emitter& out, const Vector4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.value.x << v.value.y << v.value.z <<v.value.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<< (YAML::Emitter& out, const Quaternion& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.value.x << v.value.y << v.value.z << v.value.w << YAML::EndSeq;
		return out;
	}
	// 编码游戏对象和组件的数据
	static void SerializeGameObject(YAML::Emitter& out, GameObject gameObject)
	{
		out << YAML::BeginMap;
		// Attributes
		if (gameObject.HasComponent<Attribute>())
		{
			auto& attribute = gameObject.GetComponent<Attribute>();
			// uuid and name
			out << YAML::Key << "GameObject" << YAML::Value << attribute.GetUUID();
			out << YAML::Key << "Name" << YAML::Value << attribute.m_Name;


			out << YAML::Key << "Attribute";
			out << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "IsActive" << YAML::Value << attribute.m_IsActive;
			out << YAML::EndMap;
		}
		// Transform
		if (gameObject.HasComponent<Transform>())
		{

			out << YAML::Key << "Transform";
			out << YAML::Value << YAML::BeginMap;
			auto& transform = gameObject.GetComponent<Transform>();
			out << YAML::Key << "Position" << YAML::Value << transform.m_Position;
			out << YAML::Key << "Rotation" << YAML::Value << transform.m_Rotation;
			out << YAML::Key << "Rotation_Angles" << YAML::Value << Math::Degrees(transform.GetEulerAngles());
			out << YAML::Key << "Scale" << YAML::Value << transform.m_Scale;
			out << YAML::EndMap;
		}
		// Camera
		if (gameObject.HasComponent<Camera>())
		{

			out << YAML::Key << "Camera";	
			out << YAML::Value << YAML::BeginMap;
			auto& camera = gameObject.GetComponent<Camera>();
			out << YAML::Key << "CameraParameters";
			out << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "CameraType" << YAML::Value << (int)camera.GetCameraType();
			out << YAML::Key << "OrthoGraphicSize" << YAML::Value << camera.m_OrthoGraphicSize;
			out << YAML::Key << "OrthoGraphicNear" << YAML::Value << camera.m_OrthoGraphicNear;
			out << YAML::Key << "OrthoGraphicFar" << YAML::Value << camera.m_OrthoGraphicFar;
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.m_PerspectiveFOV;
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.m_PerspectiveNear;
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.m_PerspectiveFar;
			out << YAML::EndMap;


			out << YAML::Key << "RenderOrder" << YAML::Value << camera.RenderOrder;
			out << YAML::Key << "isPrimary" << YAML::Value << camera.isPrimary;
			out << YAML::EndMap;
		}
		// Image Renderer
		if (gameObject.HasComponent<ImageRenderer>())
		{

			out << YAML::Key << "ImageRenderer";
			out << YAML::Value << YAML::BeginMap;
			auto& imageRenderer = gameObject.GetComponent<ImageRenderer>();
			out << YAML::Key << "Color" << YAML::Value << imageRenderer.m_Color;
			out << YAML::Key << "Texture" << YAML::Value << (imageRenderer.m_Texture ? imageRenderer.m_Texture->GetPath() : "None");
			out << YAML::Key << "Tiling" << YAML::Value << imageRenderer.m_Tiling;
			out << YAML::EndMap;
		}
		// Circle Renderer
		if (gameObject.HasComponent<CircleRenderer>())
		{

			out << YAML::Key << "CircleRenderer";
			out << YAML::Value << YAML::BeginMap;
			auto& circleRenderer = gameObject.GetComponent<CircleRenderer>();
			out << YAML::Key << "Color" << YAML::Value << circleRenderer.m_Color;
			out << YAML::Key << "Texture" << YAML::Value << (circleRenderer.m_Texture ? circleRenderer.m_Texture->GetPath() : "None");
			out << YAML::Key << "Radius" << YAML::Value << circleRenderer.m_Radius;
			out << YAML::Key << "Thickness" << YAML::Value << circleRenderer.m_Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRenderer.m_Fade;
			out << YAML::EndMap;
		}
		// RigidBody2D
		if (gameObject.HasComponent<RigidBody2D>())
		{
			out << YAML::Key << "RigidBody2D";
			out << YAML::Value << YAML::BeginMap;
			auto& rigidBody2D = gameObject.GetComponent<RigidBody2D>();
			out << YAML::Key << "RigidBodyType" << YAML::Value << (int)rigidBody2D.m_RigidBodyType;
			out << YAML::Key << "FixedRotation" << YAML::Value << rigidBody2D.m_FixedRotation;
			out << YAML::EndMap;
		}
		// BoxCollider2D
		if (gameObject.HasComponent<BoxCollider2D>())
		{
			out << YAML::Key << "BoxCollider2D";
			out << YAML::Value << YAML::BeginMap;
			auto& boxCollider2D = gameObject.GetComponent<BoxCollider2D>();
			out << YAML::Key << "Offset" << YAML::Value << boxCollider2D.m_Offset;
			out << YAML::Key << "Size" << YAML::Value << boxCollider2D.m_Size;
			out << YAML::Key << "Rotation" << YAML::Value << boxCollider2D.m_Rotation;
			out << YAML::Key << "Density" << YAML::Value << boxCollider2D.m_Density;
			out << YAML::Key << "Friction" << YAML::Value << boxCollider2D.m_Friction;
			out << YAML::Key << "Restitution" << YAML::Value << boxCollider2D.m_Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxCollider2D.m_RestitutionThreshold;
			out << YAML::Key << "IsTrigger" << YAML::Value << boxCollider2D.m_IsTrigger;
			out << YAML::EndMap;
		}
		// CircleCollider2D
		if (gameObject.HasComponent<CircleCollider2D>())
		{
			out << YAML::Key << "CircleCollider2D";
			out << YAML::Value << YAML::BeginMap;
			auto& circleCollider2D = gameObject.GetComponent<CircleCollider2D>();
			out << YAML::Key << "Offset" << YAML::Value << circleCollider2D.m_Offset;
			out << YAML::Key << "Radius" << YAML::Value << circleCollider2D.m_Radius;
			out << YAML::Key << "Density" << YAML::Value << circleCollider2D.m_Density;
			out << YAML::Key << "Friction" << YAML::Value << circleCollider2D.m_Friction;
			out << YAML::Key << "Restitution" << YAML::Value << circleCollider2D.m_Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << circleCollider2D.m_RestitutionThreshold;
			out << YAML::Key << "IsTrigger" << YAML::Value << circleCollider2D.m_IsTrigger;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;
	}

	template<typename T>
	static void Serializer::Serialize(const std::string& filepath, Ref<T>& data)
	{
		static_assert(false);
	}
	// 编码场景的YAML数据
	template <>
	static void GENGINE_API Serializer::Serialize(const std::string& filepath, Ref<Scene>& scene)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << scene->name;
		out << YAML::Key << "GameObjects";
		out << YAML::Value << YAML::BeginSeq;
		auto view = scene->m_Registry.view<Attribute>();
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

	template<typename T>
	static void Serializer::Deserialize(const std::string& filepath, Ref<T>& data)
	{
		static_assert(false);
	}
	// 解码场景的YAML数据
	template <>
	static void GENGINE_API Serializer::Deserialize(const std::string& filepath, Ref<Scene>& scene)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch(YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load scene file: {0}, \n{1}", filepath, e.what());
			return;
		}
		if (!data["Scene"])
			return;

		std::string sceneName = data["Scene"].as<std::string>();
		scene->name = sceneName;
		auto gameObjects = data["GameObjects"];
		if (gameObjects)
		{
			for (auto gameObject : gameObjects)
			{
				uint64_t uuid = gameObject["GameObject"].as<uint64_t>();
				std::string name;
				if (gameObject["Name"])
				{
					name = gameObject["Name"].as<std::string>();
				}
				GameObject newGameObject = scene->CreateGameObject(uuid, name);
				if (gameObject["Attribute"])
				{
					auto attribute = gameObject["Attribute"];
					bool isActive = attribute["IsActive"].as<bool>();
					newGameObject.GetComponent<Attribute>().m_IsActive = isActive;
				}
				if (gameObject["Transform"])
				{
					auto transform = gameObject["Transform"];
					auto position = transform["Position"].as<Vector3>();
					auto rotation = transform["Rotation"].as<Quaternion>();
					auto rotation_angles = transform["Rotation_Angles"].as<Vector3>();
					auto scale = transform["Scale"].as<Vector3>();
					newGameObject.GetComponent<Transform>().SetPosition(position);
					newGameObject.GetComponent<Transform>().SetEulerAngle(rotation_angles);
					newGameObject.GetComponent<Transform>().SetRotation(rotation);
					newGameObject.GetComponent<Transform>().SetScale(scale);
				}
				if (gameObject["Camera"])
				{
					auto camera = gameObject["Camera"];
					auto cameraParameters = camera["CameraParameters"];
					CameraType cameraType = (CameraType)cameraParameters["CameraType"].as<int>();
					float orthoGraphicSize = cameraParameters["OrthoGraphicSize"].as<float>();
					float orthoGraphicNear = cameraParameters["OrthoGraphicNear"].as<float>();
					float orthoGraphicFar = cameraParameters["OrthoGraphicFar"].as<float>();
					float perspectiveFOV = cameraParameters["PerspectiveFOV"].as<float>();
					float perspectiveNear = cameraParameters["PerspectiveNear"].as<float>();
					float perspectiveFar = cameraParameters["PerspectiveFar"].as<float>();
					newGameObject.AddComponent<Camera>(cameraType);
					newGameObject.GetComponent<Camera>().m_OrthoGraphicSize = orthoGraphicSize;
					newGameObject.GetComponent<Camera>().m_OrthoGraphicNear = orthoGraphicNear;
					newGameObject.GetComponent<Camera>().m_OrthoGraphicFar = orthoGraphicFar;
					newGameObject.GetComponent<Camera>().m_PerspectiveFOV = perspectiveFOV;
					newGameObject.GetComponent<Camera>().m_PerspectiveNear = perspectiveNear;
					newGameObject.GetComponent<Camera>().m_PerspectiveFar = perspectiveFar;
					newGameObject.GetComponent<Camera>().RenderOrder = camera["RenderOrder"].as<int>();
					newGameObject.GetComponent<Camera>().isPrimary = camera["isPrimary"].as<bool>();
				}
				if (gameObject["ImageRenderer"])
				{
					auto imageRenderer = gameObject["ImageRenderer"];
					Vector4 color = imageRenderer["Color"].as<Vector4>();
					Vector2 tiling = imageRenderer["Tiling"].as<Vector2>();
					std::string texturePath = imageRenderer["Texture"].as<std::string>();
					newGameObject.AddComponent<ImageRenderer>();
					newGameObject.GetComponent<ImageRenderer>().m_Color = color;
					if (texturePath != "None")
					{
						newGameObject.GetComponent<ImageRenderer>().m_Texture = Texture2D::Create(texturePath);
					}
					newGameObject.GetComponent<ImageRenderer>().m_Tiling = tiling;
				}
				if (gameObject["CircleRenderer"])
				{
					auto circleRenderer = gameObject["CircleRenderer"];
					Vector4 color = circleRenderer["Color"].as<Vector4>();
					std::string texturePath = circleRenderer["Texture"].as<std::string>();
					float radius = circleRenderer["Radius"].as<float>();
					float thickness = circleRenderer["Thickness"].as<float>();
					float fade = circleRenderer["Fade"].as<float>();
					newGameObject.AddComponent<CircleRenderer>();
					newGameObject.GetComponent<CircleRenderer>().m_Color = color;
					if (texturePath != "None")
					{
						newGameObject.GetComponent<CircleRenderer>().m_Texture = Texture2D::Create(texturePath);
					}
					newGameObject.GetComponent<CircleRenderer>().m_Radius = radius;
					newGameObject.GetComponent<CircleRenderer>().m_Thickness = thickness;
					newGameObject.GetComponent<CircleRenderer>().m_Fade = fade;
				}
				if (gameObject["RigidBody2D"])
				{
					auto rigidBody2D = gameObject["RigidBody2D"];
					RigidBody2D::RigidBodyType rigidBodyType = (RigidBody2D::RigidBodyType)rigidBody2D["RigidBodyType"].as<int>();
					bool fixedRotation = rigidBody2D["FixedRotation"].as<bool>();
					newGameObject.AddComponent<RigidBody2D>(rigidBodyType);
					newGameObject.GetComponent<RigidBody2D>().m_FixedRotation = fixedRotation;
				}
				if (gameObject["BoxCollider2D"])
				{
					auto boxCollider2D = gameObject["BoxCollider2D"];
					Vector2 offset = boxCollider2D["Offset"].as<Vector2>();
					Vector2 size = boxCollider2D["Size"].as<Vector2>();
					float rotation = boxCollider2D["Rotation"].as<float>();
					float density = boxCollider2D["Density"].as<float>();
					float friction = boxCollider2D["Friction"].as<float>();
					float restitution = boxCollider2D["Restitution"].as<float>();
					float restitutionThreshold = boxCollider2D["RestitutionThreshold"].as<float>();
					bool isTrigger = boxCollider2D["IsTrigger"].as<bool>();
					newGameObject.AddComponent<BoxCollider2D>(size, offset);
					newGameObject.GetComponent<BoxCollider2D>().m_Rotation = rotation;
					newGameObject.GetComponent<BoxCollider2D>().m_Density = density;
					newGameObject.GetComponent<BoxCollider2D>().m_Friction = friction;
					newGameObject.GetComponent<BoxCollider2D>().m_Restitution = restitution;
					newGameObject.GetComponent<BoxCollider2D>().m_RestitutionThreshold = restitutionThreshold;
					newGameObject.GetComponent<BoxCollider2D>().m_IsTrigger = isTrigger;
				}
				if (gameObject["CircleCollider2D"])
				{
					auto circleCollider2D = gameObject["CircleCollider2D"];
					Vector2 offset = circleCollider2D["Offset"].as<Vector2>();
					float radius = circleCollider2D["Radius"].as<float>();
					float density = circleCollider2D["Density"].as<float>();
					float friction = circleCollider2D["Friction"].as<float>();
					float restitution = circleCollider2D["Restitution"].as<float>();
					float restitutionThreshold = circleCollider2D["RestitutionThreshold"].as<float>();
					bool isTrigger = circleCollider2D["IsTrigger"].as<bool>();
					newGameObject.AddComponent<CircleCollider2D>(offset, radius);
					newGameObject.GetComponent<CircleCollider2D>().m_Density = density;
					newGameObject.GetComponent<CircleCollider2D>().m_Friction = friction;
					newGameObject.GetComponent<CircleCollider2D>().m_Restitution = restitution;
					newGameObject.GetComponent<CircleCollider2D>().m_RestitutionThreshold = restitutionThreshold;
					newGameObject.GetComponent<CircleCollider2D>().m_IsTrigger = isTrigger;
				}
			}
		}
	}
}
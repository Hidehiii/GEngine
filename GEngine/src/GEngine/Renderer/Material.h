#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/Texture.h"



namespace GEngine
{
	enum class Material_CullMode
	{
		None = 0,
		Front = 1,
		Back = 2
	};
	enum class Material_BlendMode
	{
		None = 0,
		Alpha = 1,
		Additive = 2,
		Multiply = 3
	};
	class GENGINE_API Material
	{
	public:
		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");

		virtual ~Material() {}

		virtual void UploadData() = 0;


		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetUInt(const std::string& name, uint32_t value) = 0;
		virtual void SetVector(const std::string& name, const Vector2& value) = 0;
		virtual void SetVector(const std::string& name, const Vector3& value) = 0;
		virtual void SetVector(const std::string& name, const Vector4& value) = 0;

		virtual float GetFloat(const std::string& name) = 0;
		virtual int GetInt(const std::string& name) = 0;
		virtual uint32_t GetUInt(const std::string& name) = 0;
		virtual Vector4 GetVector(const std::string& name) = 0;

		virtual Ref<Shader>& GetShader() = 0;
		virtual std::string GetName() = 0;

		virtual void SetMatrix4x4(const std::string& name, const Matrix4x4& value) = 0;

		virtual void SetTexture2D(const std::string& name, const Ref<Texture2D>& texture) = 0;

		virtual std::vector<ShaderUniform>& GetUniforms() = 0;
	};
}



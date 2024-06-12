#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Material.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "GEngine/Core/Buffer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(const Ref<Shader>& shader, const std::string& name = "");
		virtual ~OpenGLMaterial() override;

		virtual void UploadData() override;

		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetUInt(const std::string& name, uint32_t value) override;
		virtual void SetVector(const std::string& name, const Vector2& value) override;
		virtual void SetVector(const std::string& name, const Vector3& value) override;
		virtual void SetVector(const std::string& name, const Vector4& value) override;

		virtual float GetFloat(const std::string& name) override;
		virtual int GetInt(const std::string& name) override;
		virtual uint32_t GetUInt(const std::string& name) override;
		virtual Vector4 GetVector(const std::string& name) override;

		virtual std::string GetName() override { return m_Name; };

		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetMatrix4x4(const std::string& name, const Matrix4x4& value) override;

		virtual void SetTexture2D(const std::string& name, const Ref<Texture2D>& texture) override;

		virtual std::vector<ShaderUniform>& GetUniforms() override { return m_Uniforms; };

	private:
		ShaderUniform& GetUniformByName(const std::string& name) const;
	private:
		Ref<OpenGLShader> m_Shader;
		Ref<OpenGLUniformBuffer> m_UniformBuffer;
		std::string m_Name;
		Material_BlendMode m_BlendMode = Material_BlendMode::None;
		Material_CullMode m_CullMode = Material_CullMode::Back;
		Buffer m_UniformStorageBuffer;
		std::vector<ShaderUniform> m_Uniforms;
	};
}



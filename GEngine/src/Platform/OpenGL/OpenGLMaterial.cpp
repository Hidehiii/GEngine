#include "GEpch.h"
#include "OpenGLMaterial.h"

namespace GEngine
{
	OpenGLMaterial::OpenGLMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<OpenGLShader>(shader);
		m_Name = name.empty() ? "New Material" : name;
		if (m_Shader)
		{
			uint32_t size = 0;
			for(auto uniform : m_Shader->GetUniforms())
			{
				size += uniform.Size;
				m_Uniforms.push_back(uniform);
			}
			m_UniformStorageBuffer.Allocate(size);
			m_UniformStorageBuffer.ZeroInitialize();
			// Create uniform buffer
			// 0 is reserved for custom uniform buffer
			m_UniformBuffer = std::dynamic_pointer_cast<OpenGLUniformBuffer>(UniformBuffer::Create(size, 0));
			if(!m_UniformBuffer)
			{
				GE_CORE_CRITICAL("Failed to create uniform buffer for material {0}!", name);
			}
		}
		else
		{
			GE_CORE_WARN("Shader of material {0} is not of type OpenGLShader!", name);
		}
	}
	OpenGLMaterial::~OpenGLMaterial()
	{
	}
	void OpenGLMaterial::UploadData()
	{
		m_Shader->Bind();
		if(m_UniformStorageBuffer.Size > 0)
			m_UniformBuffer->SetData(m_UniformStorageBuffer.ReadBytes(m_UniformStorageBuffer.GetSize()), m_UniformStorageBuffer.GetSize());
	}
	void OpenGLMaterial::SetFloat(const std::string& name, float value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformStorageBuffer.Write((const void*)&value, uniform.Size, uniform.Location);
		}
	}
	void OpenGLMaterial::SetInt(const std::string& name, int value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformStorageBuffer.Write((const void*)&value, uniform.Size, uniform.Location);
		}
	}
	void OpenGLMaterial::SetUInt(const std::string& name, uint32_t value)
	{
		GE_CORE_CRITICAL("暂时没有uint类型");
	}
	void OpenGLMaterial::SetVector(const std::string& name, const Vector2& value)
	{
		SetVector(name, Vector4(value));
	}
	void OpenGLMaterial::SetVector(const std::string& name, const Vector3& value)
	{
		SetVector(name, Vector4(value));
	}
	void OpenGLMaterial::SetVector(const std::string& name, const Vector4& value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformStorageBuffer.Write((const void*)value.ValuePtr(), uniform.Size, uniform.Location);
		}
	}
	float OpenGLMaterial::GetFloat(const std::string& name)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			float value;
			value = m_UniformStorageBuffer.Read<float>(uniform.Location, uniform.Size);
			return value;
		}
		return 0.0f;
	}
	int OpenGLMaterial::GetInt(const std::string& name)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			int value;
			value = m_UniformStorageBuffer.Read<int>(uniform.Location, uniform.Size);
		}
		return 0;
	}
	uint32_t OpenGLMaterial::GetUInt(const std::string& name)
	{
		GE_CORE_CRITICAL("暂时还没有写uint");
		return 0;
	}
	Vector4 OpenGLMaterial::GetVector(const std::string& name)
	{
		ShaderUniform& uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			Vector4 value;
			value = m_UniformStorageBuffer.Read<Vector4>(uniform.Location, uniform.Size);
			return value;
		}
		return Vector4();
	}
	void OpenGLMaterial::SetName(const std::string& name)
	{
		m_Name = name;
	}
	void OpenGLMaterial::SetShader(const Ref<Shader>& shader)
	{
		GE_CORE_CRITICAL("暂时还没有写材质的Shader更换");
	}
	void OpenGLMaterial::SetMatrix4x4(const std::string& name, const Matrix4x4& value)
	{
		
	}
	void OpenGLMaterial::SetTexture2D(const std::string& name, const Ref<Texture2D>& texture)
	{
	}
	ShaderUniform OpenGLMaterial::GetUniformByName(const std::string& name) const
	{
		for(auto uniform : m_Uniforms)
		{
			if(uniform.Name == name)
				return uniform;
		}
		GE_CORE_CRITICAL("There is no uniform with name {0} in the shader!", name);
		GE_CORE_ASSERT(false, "");
		return ShaderUniform();
	}
}
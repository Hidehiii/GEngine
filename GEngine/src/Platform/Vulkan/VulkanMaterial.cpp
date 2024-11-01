#include "GEpch.h"
#include "VulkanMaterial.h"
#include "Platform/Vulkan/VulkanUtils.h"

namespace GEngine
{
	VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<VulkanShader>(shader);
		m_Name = name.empty() ? "New Material" : name;
		if (m_Shader)
		{
			uint32_t size = 0;
			for (auto uniform : m_Shader->GetUniforms())
			{
				size += uniform.Size;
				m_Uniforms.push_back(uniform);
			}
			m_UniformStorageBuffer.Allocate(size);
			m_UniformStorageBuffer.ZeroInitialize();
			// Create uniform buffer
			// 0 is reserved for custom uniform buffer
			m_UniformBuffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(UniformBuffer::Create(size, 0));
			if (!m_UniformBuffer)
			{
				GE_CORE_CRITICAL("Failed to create uniform buffer for material {0}!", name);
			}
			// Read blend type and factor
			m_BlendMode					= (Material_BlendMode)m_Shader->GetBlendMode();
			m_BlendSourceFactor			= m_Shader->GetBlendSourceFactor();
			m_BlendDestinationFactor	= m_Shader->GetBlendDestinationFactor();
			// Read depth test and depth mask
			m_EnableDepthWrite			= m_Shader->GetEnableDepthWrite();
			m_EnableDepthTest			= m_Shader->GetEnableDepthTest();
		}
		else
		{
			GE_CORE_WARN("Shader of material {0} is not of type OpenGLShader!", name);
		}
	}
	VulkanMaterial::~VulkanMaterial()
	{
	}
	void VulkanMaterial::UploadData()
	{
		if(m_UniformStorageBuffer.Size > 0)
			m_UniformBuffer->SetData(m_UniformStorageBuffer.ReadBytes(m_UniformStorageBuffer.GetSize()), m_UniformStorageBuffer.GetSize());
	}
	void VulkanMaterial::SetCullMode(Material_CullMode mode)
	{
		m_CullMode = mode;
	}
	void VulkanMaterial::SetBlendMode(Material_BlendMode mode, uint32_t source, uint32_t dest)
	{
		m_BlendMode = mode;
		m_BlendSourceFactor = source;
		m_BlendDestinationFactor = dest;
	}
	void VulkanMaterial::SetFloat(const std::string& name, float value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformStorageBuffer.Write((const void*)&value, uniform.Size, uniform.Location);
		}
	}
	void VulkanMaterial::SetInt(const std::string& name, int value)
	{
	}
	void VulkanMaterial::SetUInt(const std::string& name, uint32_t value)
	{
	}
	void VulkanMaterial::SetVector(const std::string& name, const Vector2& value)
	{
	}
	void VulkanMaterial::SetVector(const std::string& name, const Vector3& value)
	{
	}
	void VulkanMaterial::SetVector(const std::string& name, const Vector4& value)
	{
	}
	void VulkanMaterial::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
	}
	float VulkanMaterial::GetFloat(const std::string& name)
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
	int VulkanMaterial::GetInt(const std::string& name)
	{
		return 0;
	}
	uint32_t VulkanMaterial::GetUInt(const std::string& name)
	{
		return 0;
	}
	Vector4 VulkanMaterial::GetVector(const std::string& name)
	{
		return Vector4();
	}
	void VulkanMaterial::SetName(const std::string& name)
	{
	}
	void VulkanMaterial::SetShader(const Ref<Shader>& shader)
	{
	}
	void VulkanMaterial::SetMatrix4x4(const std::string& name, const Matrix4x4& value)
	{
	}
	void VulkanMaterial::SetTexture2D(const std::string& name, const Ref<Texture2D>& texture)
	{
	}
	ShaderUniform VulkanMaterial::GetUniformByName(const std::string& name) const
	{
		for (auto uniform : m_Uniforms)
		{
			if (uniform.Name == name)
				return uniform;
		}
		GE_CORE_CRITICAL("There is no uniform with name {0} in the shader!", name);
		GE_CORE_ASSERT(false);
		return ShaderUniform();
	}
}
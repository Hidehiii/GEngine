#include "GEpch.h"
#include "Material.h"
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLMaterial.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace GEngine
{
    Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name)
    {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLMaterial>(shader, name);
		}
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanMaterial>(shader, name);
			return nullptr;
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
    Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name)
    {
        return nullptr;;
    }
	ShaderUniform Material::GetUniformByName(const std::string& name) const
	{
		for (auto uniform : m_Uniforms)
		{
			if (uniform.Name == name)
				return uniform;
		}
		GE_CORE_ASSERT(false, "There is no uniform with name {0} in the shader!", name);
		return ShaderUniform();
	}
	ShaderUniformTexture2D& Material::GetUniformTexture2DByName(const std::string& name)
	{
		for (int i = 0; i < m_Texture2D.size(); i++)
		{
			if (m_Texture2D.at(i).Name == name)
			{
				return (m_Texture2D.at(i));
			}
		}
		GE_CORE_ASSERT(false, "There is no uniform texture2D with name {0} in the shader!", name);
		return ShaderUniformTexture2D();
	}
	ShaderUniformStorageImage2D& Material::GetUniformStorageImage2DByName(const std::string& name)
	{
		for (int i = 0; i < m_StorageImage2D.size(); i++)
		{
			if (m_StorageImage2D.at(i).Name == name)
			{
				return m_StorageImage2D.at(i);
			}
		}
		GE_CORE_ASSERT(false, "There is no uniform storage image2d with name {0} in the shader!", name);
		return ShaderUniformStorageImage2D();
	}
	ShaderUniformStorageBuffer& Material::GetUniformStorageBufferByName(const std::string& name)
	{
		for (int i = 0; i < m_StorageBuffer.size(); i++)
		{
			if (m_StorageBuffer.at(i).Name == name)
			{
				return (m_StorageBuffer.at(i));
			}
		}
		GE_CORE_ASSERT(false, "There is no uniform storage buffer with name {0} in the shader!", name);
		return ShaderUniformStorageBuffer();
	}
	void Material::SetCullMode(CullMode mode)
	{
		m_CullMode = mode;
	}
	void Material::SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest)
	{
		m_BlendMode = mode;
		m_BlendSourceFactor = source;
		m_BlendDestinationFactor = dest;
	}
	void Material::SetFloat(const std::string& name, float value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformsBuffer.Write((const void*)&value, uniform.Size, uniform.Location);
		}
	}
	void Material::SetInt(const std::string& name, int value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformsBuffer.Write((const void*)&value, uniform.Size, uniform.Location);
		}
	}
	void Material::SetUInt(const std::string& name, uint32_t value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformsBuffer.Write((const void*)&value, uniform.Size, uniform.Location);
		}
	}
	void Material::SetVector(const std::string& name, const Vector2& value)
	{
		SetVector(name, Vector4(value, 0.0f, 0.0f));
	}
	void Material::SetVector(const std::string& name, const Vector3& value)
	{
		SetVector(name, Vector4(value, 0.0f));
	}
	void Material::SetVector(const std::string& name, const Vector4& value)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			m_UniformsBuffer.Write((const void*)Math::ValuePtr(value), uniform.Size, uniform.Location);
		}
	}
	float Material::GetFloat(const std::string& name)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			float value;
			value = m_UniformsBuffer.Read<float>(uniform.Location, uniform.Size);
			return value;
		}
		return 0.0f;
	}
	int Material::GetInt(const std::string& name)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			int value;
			value = m_UniformsBuffer.Read<int>(uniform.Location, uniform.Size);
		}
		return 0;
	}
	uint32_t Material::GetUInt(const std::string& name)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			int value;
			value = m_UniformsBuffer.Read<uint32_t>(uniform.Location, uniform.Size);
		}
		return uint32_t();
	}
	Vector4 Material::GetVector(const std::string& name)
	{
		ShaderUniform& uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			Vector4 value;
			value = m_UniformsBuffer.Read<Vector4>(uniform.Location, uniform.Size);
			return value;
		}
		return Vector4();
	}
	void Material::SetTexture2D(const std::string& name, const Ref<Texture2D>& texture)
	{
		ShaderUniformTexture2D& uniform = GetUniformTexture2DByName(name);
		uniform.Texture = texture;
	}
	Ref<Texture2D> Material::GetTexture2D(const std::string& name)
	{
		return GetUniformTexture2DByName(name).Texture;
	}
	void Material::SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage)
	{
		ShaderUniformStorageImage2D& uniform = GetUniformStorageImage2DByName(name);
		uniform.Image = storageImage;
	}
	Ref<StorageImage2D> Material::GetStorageImage2D(const std::string& name)
	{
		return GetUniformStorageImage2DByName(name).Image;
	}
	void Material::SetStorageBuffer(const std::string& name, const Ref<StorageBuffer>& storageBuffer)
	{
		ShaderUniformStorageBuffer& uniform = GetUniformStorageBufferByName(name);
		uniform.Buffer = storageBuffer;
	}
	Ref<StorageBuffer> Material::GetStorageBuffer(const std::string& name)
	{
		return GetUniformStorageBufferByName(name).Buffer;
	}
}
#include "GEpch.h"
#include "Material.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsAPI.h"
#include "Platform/OpenGL/OpenGLMaterial.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace GEngine
{
    Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name)
    {
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::API::OpenGL: {
			return CreateRef<OpenGLMaterial>(shader, name);
		}
		case GraphicsAPI::API::Vulkan: {
			return CreateRef<VulkanMaterial>(shader, name);
			return nullptr;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
        return nullptr;
    }
	Ref<Material> Material::Create(const std::string& shaderPath, const std::string& name)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::API::OpenGL: {
			return CreateRef<OpenGLMaterial>(Shader::Create(shaderPath), name);
		}
		case GraphicsAPI::API::Vulkan: {
			return CreateRef<VulkanMaterial>(Shader::Create(shaderPath), name);
			return nullptr;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
    Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name)
    {
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::API::OpenGL: {
			return CreateRef<OpenGLMaterial>(Shader::Create(other->GetShader()->GetShaderPath()), name);
		}
		case GraphicsAPI::API::Vulkan: {
			return CreateRef<VulkanMaterial>(Shader::Create(other->GetShader()->GetShaderPath()), name);
			return nullptr;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
    }
	void Material::SetEnableDepthWrite(bool enabled, const int& pass)
	{
		m_Passes.at(pass).State.DepthWrite = enabled;
	}
	void Material::SetDepthTestOp(CompareOperation op, const int& pass)
	{
		m_Passes.at(pass).State.DepthTestOp = op;
	}
	void Material::SetCullMode(CullMode mode, const int& pass)
	{
		m_Passes.at(pass).State.Cull = mode;
	}
	void Material::SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, const int& pass)
	{
		m_Passes.at(pass).State.BlendColor = modeColor;
		m_Passes.at(pass).State.BlendAlpha = modeAlpha;
		m_Passes.at(pass).State.BlendColorSrc = srcColor;
		m_Passes.at(pass).State.BlendAlphaSrc = srcAlpha;
		m_Passes.at(pass).State.BlendColorDst = dstColor;
		m_Passes.at(pass).State.BlendAlphaDst = dstAlpha;
	}
	void Material::SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest, const int& pass)
	{
		m_Passes.at(pass).State.BlendColor = mode;
		m_Passes.at(pass).State.BlendAlpha = mode;
		m_Passes.at(pass).State.BlendColorSrc = source;
		m_Passes.at(pass).State.BlendAlphaSrc = source;
		m_Passes.at(pass).State.BlendColorDst = dest;
		m_Passes.at(pass).State.BlendAlphaDst = dest;
	}
	void Material::SetFloat(const std::string& name, float value)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((float*)m_Properties.at(name)) = value;
		WriteConstProperty(name, (const void*)&value);
	}
	void Material::SetInt(const std::string& name, int value)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((int*)m_Properties.at(name)) = value;
		WriteConstProperty(name, (const void*)&value);
	}
	void Material::SetUInt(const std::string& name, uint32_t value)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((uint32_t*)m_Properties.at(name)) = value;
		WriteConstProperty(name, (const void*)&value);
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
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Vector4*)m_Properties.at(name)) = value;
		WriteConstProperty(name, (const void*)&value);
	}
	void Material::SetMatrix4x4(const std::string& name, const Matrix4x4& value)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Matrix4x4*)m_Properties.at(name)) = value;
		WriteConstProperty(name, (const void*)&value);
	}
	float Material::GetFloat(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((float*)m_Properties.at(name));
	}
	int Material::GetInt(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((int*)m_Properties.at(name));
	}
	uint32_t Material::GetUInt(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((uint32_t*)m_Properties.at(name));
	}
	Vector4 Material::GetVector(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((Vector4*)m_Properties.at(name));
	}
	void Material::SetTexture2D(const std::string& name, const Ref<Texture2D>& texture)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Ref<Texture2D>*)m_Properties.at(name)) = texture;
		WriteReferenceProperty(name, (void*)&texture);
	}
	const Ref<Texture2D> Material::GetTexture2D(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((Ref<Texture2D>*)m_Properties.at(name));
	}
	void Material::SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Ref<StorageImage2D>*)m_Properties.at(name)) = storageImage;
		WriteReferenceProperty(name, (void*)&storageImage);
	}
	const Ref<StorageImage2D> Material::GetStorageImage2D(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((Ref<StorageImage2D>*)m_Properties.at(name));
	}
	void Material::SetStorageBuffer(const std::string& name, const Ref<StorageBuffer>& storageBuffer)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Ref<StorageBuffer>*)m_Properties.at(name)) = storageBuffer;
		WriteReferenceProperty(name, (void*)&storageBuffer);
	}
	const Ref<StorageBuffer> Material::GetStorageBuffer(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((Ref<StorageBuffer>*)m_Properties.at(name));
	}
	void Material::SetCubeMap(const std::string& name, const Ref<CubeMap>& cubeMap)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Ref<CubeMap>*)m_Properties.at(name)) = cubeMap;
		WriteReferenceProperty(name, (void*)&cubeMap);
	}
	const Ref<CubeMap> Material::GetCubeMap(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((Ref<CubeMap>*)m_Properties.at(name));
	}
	void Material::SetTexture2DArray(const std::string& name, const Ref<Texture2DArray>& array)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		*((Ref<Texture2DArray>*)m_Properties.at(name)) = array;
		WriteReferenceProperty(name, (void*)&array);
	}
	const Ref<Texture2DArray> Material::GetTexture2DArray(const std::string& name)
	{
		GE_CORE_ASSERT(m_Properties.find(name) != m_Properties.end(), "Property does not exist!");
		return *((Ref<Texture2DArray>*)m_Properties.at(name));
	}
	uint32_t Material::InitializePropertiesMemory()
	{
		GE_CORE_ASSERT(m_Shader, "Shader ref is NULL!");
		uint32_t size = 0;
		auto& peroperties = m_Shader->GetProperties();
		for (auto& [name, type] : peroperties)
		{
			size += Utils::GetShaderPropertyTypeSize(type);
		}
		m_PropertiesMemory.Allocate(size);
		m_PropertiesMemory.ZeroInitialize();
		uint32_t offset = 0;
		for (auto& [name, type] : peroperties)
		{
			m_Properties[name] = (void*)((uint32_t)m_PropertiesMemory.ReadBytes() + offset);
			offset += Utils::GetShaderPropertyTypeSize(type);
		}
		return size;
	}
	void Material::WriteConstProperty(const std::string& name, const void* value)
	{
		for (auto& pass : m_Passes)
		{
			if (pass.ConstPropertiesDesc.find(name) != pass.ConstPropertiesDesc.end())
			{
				pass.ConstProperties.Write(value, pass.ConstPropertiesDesc.at(name).Size, pass.ConstPropertiesDesc.at(name).Location);
			}
		}
	}
	void Material::WriteReferenceProperty(const std::string& name, void* ptr)
	{
		for (auto& pass : m_Passes)
		{
			if (pass.ReferenceProperties.find(name) != pass.ReferenceProperties.end())
			{
				pass.ReferenceProperties.at(name).Ptr = ptr;
			}
		}
	}
}
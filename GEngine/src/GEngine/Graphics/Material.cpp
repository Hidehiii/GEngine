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
	ShaderUniformCubeMap& Material::GetUniformCubeMapByName(const std::string& name)
	{
		for (int i = 0; i < m_CubeMap.size(); i++)
		{
			if (m_CubeMap.at(i).Name == name)
			{
				return (m_CubeMap.at(i));
			}
		}
		GE_CORE_ASSERT(false, "There is no uniform cube map with name {0} in the shader!", name);
		return ShaderUniformCubeMap();
	}
	ShaderUniformTexture2DArray& Material::GetUniformTexture2DArrayByName(const std::string& name)
	{
		for (int i = 0; i < m_Texture2DArray.size(); i++)
		{
			if (m_Texture2DArray.at(i).Name == name)
			{
				return m_Texture2DArray.at(i);
			}
		}
		GE_CORE_ASSERT(false, "There is no uniform texture2D array with name {} in shader ", name);
		return ShaderUniformTexture2DArray();
	}
	void Material::SetCullMode(CullMode mode)
	{
		m_CullMode = mode;
	}
	void Material::SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
	{
		m_BlendModeColor = modeColor;
		m_BlendModeAlpha = modeAlpha;
		m_BlendColorSourceFactor = srcColor;
		m_BlendColorDestinationFactor = dstColor;
		m_BlendAlphaSourceFactor = srcAlpha;
		m_BlendAlphaDestinationFactor = dstAlpha;
	}
	void Material::SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest)
	{
		m_BlendModeColor = mode;
		m_BlendModeAlpha = mode;
		m_BlendColorSourceFactor = source;
		m_BlendAlphaSourceFactor = source;
		m_BlendColorDestinationFactor = dest;
		m_BlendAlphaDestinationFactor = dest;
	}
	void Material::SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest, const std::string& pass)
	{
		m_RenderStates[pass].BlendColor		= mode;
		m_RenderStates[pass].BlendAlpha		= mode;
		m_RenderStates[pass].BlendColorSrc	= source;
		m_RenderStates[pass].BlendAlphaSrc	= source;
		m_RenderStates[pass].BlendColorDst	= dest;
		m_RenderStates[pass].BlendAlphaDst	= dest;
	}
	void Material::SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, const std::string& pass)
	{
		m_RenderStates[pass].BlendColor		= modeColor;
		m_RenderStates[pass].BlendAlpha		= modeAlpha;
		m_RenderStates[pass].BlendColorSrc	= srcColor;
		m_RenderStates[pass].BlendColorDst	= dstColor;
		m_RenderStates[pass].BlendAlphaSrc	= srcAlpha;
		m_RenderStates[pass].BlendAlphaDst	= dstAlpha;
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
	void Material::SetMatrix4x4(const std::string& name, const Matrix4x4& value)
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
			value = m_UniformsBuffer.Read<float>(uniform.Location);
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
			value = m_UniformsBuffer.Read<int>(uniform.Location);
			return value;
		}
		return 0;
	}
	uint32_t Material::GetUInt(const std::string& name)
	{
		ShaderUniform uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			int value;
			value = m_UniformsBuffer.Read<uint32_t>(uniform.Location);
			return value;
		}
		return uint32_t();
	}
	Vector4 Material::GetVector(const std::string& name)
	{
		ShaderUniform& uniform = GetUniformByName(name);
		if (uniform.Size)
		{
			Vector4 value;
			value = m_UniformsBuffer.Read<Vector4>(uniform.Location);
			return value;
		}
		return Vector4();
	}
	void Material::SetTexture2D(const std::string& name, const Ref<Texture2D>& texture)
	{
		ShaderUniformTexture2D& uniform = GetUniformTexture2DByName(name);
		uniform.Texture = texture;
	}
	void Material::SetTexture2D(const int index, const Ref<Texture2D>& texture)
	{
		m_Texture2D.at(index).Texture = texture;
	}
	const Ref<Texture2D> Material::GetTexture2D(const std::string& name)
	{
		return GetUniformTexture2DByName(name).Texture;
	}
	void Material::SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage)
	{
		ShaderUniformStorageImage2D& uniform = GetUniformStorageImage2DByName(name);
		uniform.Image = storageImage;
	}
	void Material::SetStorageImage2D(const int index, const Ref<StorageImage2D>& storageImage)
	{
		m_StorageImage2D.at(index).Image = storageImage;
	}
	const Ref<StorageImage2D> Material::GetStorageImage2D(const std::string& name)
	{
		return GetUniformStorageImage2DByName(name).Image;
	}
	void Material::SetStorageBuffer(const std::string& name, const Ref<StorageBuffer>& storageBuffer)
	{
		ShaderUniformStorageBuffer& uniform = GetUniformStorageBufferByName(name);
		uniform.Buffer = storageBuffer;
	}
	void Material::SetStorageBuffer(const int index, const Ref<StorageBuffer>& storageBuffer)
	{
		m_StorageBuffer.at(index).Buffer = storageBuffer;
	}
	const Ref<StorageBuffer> Material::GetStorageBuffer(const std::string& name)
	{
		return GetUniformStorageBufferByName(name).Buffer;
	}
	void Material::SetCubeMap(const std::string& name, const Ref<CubeMap>& cubeMap)
	{
		ShaderUniformCubeMap& uniform = GetUniformCubeMapByName(name);
		uniform.Cubemap = cubeMap;
	}
	void Material::SetCubeMap(const int index, const Ref<CubeMap>& cubeMap)
	{
		m_CubeMap.at(index).Cubemap = cubeMap;
	}
	void Material::SetCubeMap(const std::string& name, uint32_t width, uint32_t height, CubeMap::CubeMapFace face, const Ref<Texture2D>& texture)
	{
		ShaderUniformCubeMap& uniform = GetUniformCubeMapByName(name);
		uniform.Cubemap->SetData(texture, width, height, face);
	}
	void Material::SetCubeMap(const int index, uint32_t width, uint32_t height, CubeMap::CubeMapFace face, const Ref<Texture2D>& texture)
	{
		m_CubeMap.at(index).Cubemap->SetData(texture, width, height, face);
	}
	const Ref<CubeMap> Material::GetCubeMap(const std::string& name)
	{
		return GetUniformCubeMapByName(name).Cubemap;
	}
	void Material::SetTexture2DArray(const std::string& name, uint32_t width, uint32_t height, const int layer, const Ref<Texture2D>& texture)
	{
		ShaderUniformTexture2DArray& uniform = GetUniformTexture2DArrayByName(name);
		uniform.TextureArray->SetData(texture, width, height, layer);
	}
	void Material::SetTexture2DArray(const int index, uint32_t width, uint32_t height, const int layer, const Ref<Texture2D>& texture)
	{
		m_Texture2DArray.at(index).TextureArray->SetData(texture, width, height, layer);
	}
	void Material::SetTexture2DArray(const std::string& name, const Ref<Texture2DArray>& array)
	{
		ShaderUniformTexture2DArray& uniform = GetUniformTexture2DArrayByName(name);
		uniform.TextureArray = array;
	}
	void Material::SetTexture2DArray(const int index, const Ref<Texture2DArray>& array)
	{
		m_Texture2DArray.at(index).TextureArray = array;
	}
	const Ref<Texture2DArray> Material::GetTexture2DArray(const std::string& name)
	{
		return GetUniformTexture2DArrayByName(name).TextureArray;
	}
}
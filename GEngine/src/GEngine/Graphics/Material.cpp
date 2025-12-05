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
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLMaterial>(shader, name);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanMaterial>(shader, name);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
        return nullptr;
    }
	Ref<Material> Material::Create(const std::string& shaderPath, const std::string& name)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLMaterial>(Shader::Create(shaderPath), name);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanMaterial>(Shader::Create(shaderPath), name);
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
    Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name)
    {
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			return CreateRef<OpenGLMaterial>(Shader::Create(other->GetShader()->GetShaderPath()), name);
		}
		case GRAPHICS_API_VULKAN: {
			return CreateRef<VulkanMaterial>(Shader::Create(other->GetShader()->GetShaderPath()), name);
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
	void Material::SetCullMode(CullMode mode, int pass)
	{
		m_Passes.at(pass).State.Cull = mode;
	}
	void Material::SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, int pass)
	{
		m_Passes.at(pass).State.BlendColor = modeColor;
		m_Passes.at(pass).State.BlendAlpha = modeAlpha;
		m_Passes.at(pass).State.BlendColorSrc = srcColor;
		m_Passes.at(pass).State.BlendAlphaSrc = srcAlpha;
		m_Passes.at(pass).State.BlendColorDst = dstColor;
		m_Passes.at(pass).State.BlendAlphaDst = dstAlpha;
	}
	void Material::SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest, int pass)
	{
		m_Passes.at(pass).State.BlendColor = mode;
		m_Passes.at(pass).State.BlendAlpha = mode;
		m_Passes.at(pass).State.BlendColorSrc = source;
		m_Passes.at(pass).State.BlendAlphaSrc = source;
		m_Passes.at(pass).State.BlendColorDst = dest;
		m_Passes.at(pass).State.BlendAlphaDst = dest;
	}
	std::vector<std::unordered_map<uint32_t, uint32_t>> Material::InitializePassPropertiesMemory()
	{
		GE_CORE_ASSERT(m_Shader, "Shader ref is NULL!");
		auto& passes = m_Shader->GetPasseReflections();
		std::vector<std::unordered_map<uint32_t, uint32_t>> sizes;
		m_Passes.clear();
		for (size_t i = 0; i < passes.size(); i++)
		{
			m_Passes.push_back(ShaderPass());
			m_Passes.at(m_Passes.size() - 1).State = passes[i].State;
			sizes.push_back(std::unordered_map<uint32_t, uint32_t>());

			for (auto& info : passes.at(i).CBuffers)
			{
				uint32_t size = 0;
				for (auto& prop : info.Constants)
				{
					ShaderConstantProperty		property;
					property.CBufferBindPoint	= info.BindPoint;
					property.PropertyLocation	= prop.Location;

#ifdef GE_DEBUG
					if(m_Passes.at(m_Passes.size() - 1).ConstPropertiesDesc.find(prop.Name) != m_Passes.at(m_Passes.size() - 1).ConstPropertiesDesc.end())
					{
						GE_CORE_WARN("Already exists constant property name '{}' in cbuffer bind point {}!", prop.Name, m_Passes.at(m_Passes.size() - 1).ConstPropertiesDesc[prop.Name].CBufferBindPoint);
						GE_CORE_WARN("Material::InitializePassPropertiesMemory: Duplicate constant property name '{}' in cbuffer '{}', bind point {}!", prop.Name, info.Name, info.BindPoint);
					}
#endif
					m_Passes.at(m_Passes.size() - 1).ConstPropertiesDesc[prop.Name] = property;
					
					size += prop.Size;
				}
				GE_CORE_ASSERT(size == info.Size, "CBuffer size mismatch!");

#ifdef GE_DEBUG
				if(m_Passes.at(m_Passes.size() - 1).CBuffers.find(info.BindPoint) != m_Passes.at(m_Passes.size() - 1).CBuffers.end())
				{
					GE_CORE_WARN("Already exists cbuffer bind point {}!", info.BindPoint);
					GE_CORE_WARN("Material::InitializePassPropertiesMemory: Duplicate cbuffer '{}' bind point {}!", info.Name, info.BindPoint);
				}
#endif
				m_Passes.at(m_Passes.size() - 1).CBuffers[info.BindPoint] = Buffer();
				m_Passes.at(m_Passes.size() - 1).CBuffers[info.BindPoint].Allocate(info.Size);
				m_Passes.at(m_Passes.size() - 1).CBuffers[info.BindPoint].ZeroInitialize();

				sizes.at(sizes.size() - 1)[info.BindPoint] = info.Size;
			}
		}
		return sizes;
	}
	void Material::WriteConstProperty(const std::string& name, const void* value, const uint32_t size)
	{
		for (auto& pass : m_Passes)
		{
			if (pass.ConstPropertiesDesc.find(name) != pass.ConstPropertiesDesc.end())
			{
				pass.CBuffers.at(pass.ConstPropertiesDesc.at(name).CBufferBindPoint).Write(value, size, pass.ConstPropertiesDesc.at(name).PropertyLocation);
			}
		}
	}
	const void* Material::ReadConstProperty(const std::string& name)
	{
		for(auto& pass : m_Passes)
		{
			if (pass.ConstPropertiesDesc.find(name) != pass.ConstPropertiesDesc.end())
			{
				return pass.CBuffers.at(pass.ConstPropertiesDesc.at(name).CBufferBindPoint).ReadBytes(pass.ConstPropertiesDesc.at(name).PropertyLocation);
			}
		}
		return nullptr;
	}
	void Material::WriteResourceProperty(const std::string& name, void* ptr)
	{
		for (auto& pass : m_Passes)
		{
			if (pass.ResourceProperties.find(name) != pass.ResourceProperties.end())
			{
				pass.ResourceProperties.at(name).Ptr = ptr;
			}
		}
	}
	void* Material::ReadResourceProperty(const std::string& name)
	{
		for (auto& pass : m_Passes)
		{
			if (pass.ResourceProperties.find(name) != pass.ResourceProperties.end())
			{
				return pass.ResourceProperties.at(name).Ptr;
			}
		}
		return nullptr;
	}
}
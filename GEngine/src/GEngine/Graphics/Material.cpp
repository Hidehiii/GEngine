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
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLMaterial>(shader, name);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
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
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLMaterial>(Shader::Create(shaderPath), name);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
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
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			return CreateRef<OpenGLMaterial>(Shader::Create(other->GetShader()->GetShaderPath()), name);
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
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
	void Material::InitializePassPropertiesMemory()
	{
		GE_CORE_ASSERT(m_Shader, "Shader ref is NULL!");
		auto& passes = m_Shader->GetPasses();
		for (size_t i = 0; i < passes.size(); i++)
		{
			m_Passes.push_back(passes[i]);
			uint32_t size = 0;
			for (auto& [name, prop] : m_Passes[i].ConstPropertiesDesc)
			{
				size += prop.Size;
			}
			m_Passes[i].ConstProperties.Allocate(size);
			m_Passes[i].ConstProperties.ZeroInitialize();
		}
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
	const void* Material::ReadConstProperty(const std::string& name)
	{
		for(auto& pass : m_Passes)
		{
			if (pass.ConstPropertiesDesc.find(name) != pass.ConstPropertiesDesc.end())
			{
				return pass.ConstProperties.ReadBytes(pass.ConstPropertiesDesc.at(name).Location);
			}
		}
		return nullptr;
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
	void* Material::ReadReferenceProperty(const std::string& name)
	{
		for (auto& pass : m_Passes)
		{
			if (pass.ReferenceProperties.find(name) != pass.ReferenceProperties.end())
			{
				return pass.ReferenceProperties.at(name).Ptr;
			}
		}
		return nullptr;
	}
}
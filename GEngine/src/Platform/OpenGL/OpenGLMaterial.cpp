#include "GEpch.h"
#include "OpenGLMaterial.h"
#include "Platform/OpenGL/OpenGLUtils.h"

namespace GEngine
{
	OpenGLMaterial::OpenGLMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<OpenGLShader>(shader);
		m_Name = name.empty() ? "New Material" : name;
		GE_CORE_ASSERT(m_Shader, "Shader is null!");
		std::vector<uint32_t> sizes = InitializePassPropertiesMemory();
		// Create uniform buffer
		// 0 is reserved for custom uniform buffer
		for(int i = 0; i < sizes.size(); i++)
		{
			if (sizes[i] > 0)
			{
				m_UniformBuffers.push_back(std::dynamic_pointer_cast<OpenGLUniformBuffer>(UniformBuffer::Create(sizes[i], 0)));
			}
			else
			{
				m_UniformBuffers.push_back(nullptr);
			}
		}
		
	}
	OpenGLMaterial::~OpenGLMaterial()
	{
	}
	void OpenGLMaterial::Update(CommandBuffer* cmdBuffer, const int& pass)
	{
		m_Shader->Use(pass);

		if (m_UniformBuffers.at(pass))
			m_UniformBuffers.at(pass)->SetData(m_Passes.at(pass).ConstProperties.ReadBytes(), m_Passes.at(pass).ConstProperties.GetSize());

		for (auto&& [name, prop] : m_Passes.at(pass).ReferenceProperties)
		{
			auto propertyTypes = GetShader()->GetProperties();
			GE_CORE_ASSERT(propertyTypes.find(name) != propertyTypes.end(), "Could not find type of property {}!", name);
			switch (propertyTypes[name])
			{
			case SHADER_PROPERTY_TYPE_SAMPLER_2D:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<Texture2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_CUBE:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<CubeMap>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_2D:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<StorageImage2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<StorageBuffer>*)prop.Ptr))->Bind(prop.Location);
				break;
			}
			default:
				break;
			}
		}

		Utils::SetCull(m_Passes.at(pass).State.Cull);
		Utils::SetBlend(m_Passes.at(pass).State.BlendColor, m_Passes.at(pass).State.BlendAlpha,
			m_Passes.at(pass).State.BlendColorSrc, m_Passes.at(pass).State.BlendColorDst, 
			m_Passes.at(pass).State.BlendAlphaSrc, m_Passes.at(pass).State.BlendAlphaDst);
		Utils::SetDepthTest(m_Passes.at(pass).State.DepthTestOp);
		Utils::EnableDepthWrite(m_Passes.at(pass).State.DepthWrite);
	}

	void OpenGLMaterial::SetShader(const Ref<Shader>& shader)
	{
		GE_CORE_CRITICAL("暂时还没有写材质的Shader更换");
	}

}
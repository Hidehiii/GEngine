#include "GEpch.h"
#include "OpenGLMaterial.h"
#include "Platform/OpenGL/OpenGLUtils.h"

namespace GEngine
{
	OpenGLMaterial::OpenGLMaterial(const Ref<Shader>& shader, const std::string& name)
	{
		m_Shader = std::dynamic_pointer_cast<OpenGLShader>(shader);
		m_Name = name.empty() ? "New Material" : name;
		if (m_Shader)
		{
			uint32_t size = InitializePropertiesMemory();
			// Create uniform buffer
			// 0 is reserved for custom uniform buffer
			if (size > 0)
			{
				m_UniformBuffer = std::dynamic_pointer_cast<OpenGLUniformBuffer>(UniformBuffer::Create(size, 0));
			}

			m_Passes = std::vector<ShaderPass>(m_Shader->GetPasses());
		}
		else
		{
			GE_CORE_WARN("Shader of material {0} is not of type OpenGLShader!", name);
		}
	}
	OpenGLMaterial::~OpenGLMaterial()
	{
	}
	void OpenGLMaterial::Update(CommandBuffer* cmdBuffer, const int& pass)
	{
		m_Shader->Use(pass);

		if (m_UniformBuffer)
			m_UniformBuffer->SetData(m_Passes.at(pass).ConstProperties.ReadBytes(), m_Passes.at(pass).ConstProperties.GetSize());

		for (auto&& [name, prop] : m_Passes.at(pass).ReferenceProperties)
		{
			auto propertyTypes = GetShader()->GetProperties();
			GE_CORE_ASSERT(propertyTypes.find(name) != propertyTypes.end(), "Could not find type of property {}!", name);
			switch (propertyTypes[name])
			{
			case ShaderPropertyType::Sampler2D:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<Texture2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case ShaderPropertyType::SamplerCube:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<CubeMap>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case ShaderPropertyType::Sampler2DArray:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Sampler:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Texture2D:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::TextureCube:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::Texture2DArray:
			{
				GE_CORE_ASSERT(false, "");
				break;
			}
			case ShaderPropertyType::StorageImage2D:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<StorageImage2D>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case ShaderPropertyType::StorageBuffer:
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
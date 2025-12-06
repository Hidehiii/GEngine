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
		std::vector<std::unordered_map<uint32_t, uint32_t>> sizes = InitializePassPropertiesMemory();
		// Create uniform buffer
		for(auto& pass : sizes)
		{
			std::unordered_map<uint32_t, Ref<OpenGLUniformBuffer>> ubuffers;
			for (auto& [bindPoint, size] : pass)
			{
				if (size > 0)
				{
					Ref<OpenGLUniformBuffer> ubo = CreateRef<OpenGLUniformBuffer>(size);
					ubuffers[bindPoint] = ubo;
				}
			}
			m_UniformBuffers.push_back(ubuffers);
		}
	}
	OpenGLMaterial::~OpenGLMaterial()
	{
	}
	void OpenGLMaterial::Update(CommandBuffer* cmdBuffer, const int& pass)
	{
		m_Shader->Use(pass);

		for (auto& [bindPoint, ubo] : m_UniformBuffers.at(pass))
		{
			GE_CORE_ASSERT(ubo, "Uniform buffer is null!");
			GE_CORE_ASSERT(m_Passes.at(pass).CBuffers.find(bindPoint) != m_Passes.at(pass).CBuffers.end(), "Uniform buffer bind point not found in pass!");
			ubo->SetBindPoint(bindPoint);
			if (ubo->IsDynamic() == false)
			{
				GE_CORE_ASSERT(m_Passes.at(pass).CBuffers.at(bindPoint).Data, "CBuffer is NULL!");
				ubo->SetData(m_Passes.at(pass).CBuffers.at(bindPoint).ReadBytes(), m_Passes.at(pass).CBuffers.at(bindPoint).GetSize());
			}
		}

		for (auto&& [name, prop] : m_Passes.at(pass).ResourceProperties)
		{
			auto type = m_Shader->GetPropertyType(name);
			switch (type)
			{
			case SHADER_PROPERTY_TYPE_SAMPLER_2D:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<Texture2DCombineSampler>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
				break;
			}
			case SHADER_PROPERTY_TYPE_SAMPLER_CUBE:
			{
				m_Shader->SetInt1(name, prop.Location, pass);
				(*((Ref<CubeMapCombineSampler>*)prop.Ptr))->Bind(cmdBuffer, prop.Location);
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
				GE_CORE_ASSERT(false, "OpenGL not support sperate image!");
				break;
			}
			case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
			{
				GE_CORE_ASSERT(false, "OpenGL not support sperate image!");
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

	Buffer OpenGLMaterial::SetUniformBuffer(const int& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf)
	{
		GE_CORE_ASSERT(m_UniformBuffers.size() > pass, "Pass index out of range!");
		auto& ubuffers = m_UniformBuffers.at(pass);
		GE_CORE_ASSERT(ubuffers.find(bindPoint) != ubuffers.end(), "Uniform buffer bind point not found!");
		ubuffers.at(bindPoint) = std::dynamic_pointer_cast<OpenGLUniformBuffer>(buf);
		ubuffers.at(bindPoint)->SetBindPoint(bindPoint);
		Buffer oldBuffer = m_Passes.at(pass).CBuffers.at(bindPoint);
		m_Passes.at(pass).CBuffers[bindPoint] = buffer;
		return oldBuffer;
	}

}
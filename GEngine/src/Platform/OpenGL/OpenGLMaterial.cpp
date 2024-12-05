#include "GEpch.h"
#include "OpenGLMaterial.h"

#include "GEngine/Renderer/RenderCommand.h"

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
			m_UniformsBuffer.Allocate(size);
			m_UniformsBuffer.ZeroInitialize();
			// Create uniform buffer
			// 0 is reserved for custom uniform buffer
			m_UniformBuffer = std::dynamic_pointer_cast<OpenGLUniformBuffer>(UniformBuffer::Create(size, 0));
			if(!m_UniformBuffer)
			{
				GE_CORE_CRITICAL("Failed to create uniform buffer for material {0}!", name);
			}
			// Read blend type and factor
			m_BlendMode					= m_Shader->GetBlendMode();
			m_BlendSourceFactor			= m_Shader->GetBlendSourceFactor();
			m_BlendDestinationFactor	= m_Shader->GetBlendDestinationFactor();
			// cull mode
			m_CullMode					= m_Shader->GetCullMode();
			// Read depth test and depth mask
			m_EnableDepthWrite			= m_Shader->GetEnableDepthWrite();
			m_EnableDepthTest			= m_Shader->GetEnableDepthTest();
			// Texture2D
			m_Texture2D					= m_Shader->GetTexture2D();
			// StorageImage2D
			m_StorageImage2D			= m_Shader->GetStorageImage2D();
			// StorageBuffer
			m_StorageBuffer				= m_Shader->GetStorageBuffer();
			// cube map
			m_CubeMap					= m_Shader->GetCubeMap();
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
		if(m_UniformsBuffer.Size > 0)
			m_UniformBuffer->SetData(m_UniformsBuffer.ReadBytes(m_UniformsBuffer.GetSize()), m_UniformsBuffer.GetSize());

		for (auto& texture2D : m_Texture2D)
		{
			m_Shader->SetInt1(texture2D.Name, texture2D.Slot);
			texture2D.Texture->Bind(texture2D.Slot);
		}

		for (auto& image2D : m_StorageImage2D)
		{
			m_Shader->SetInt1(image2D.Name, image2D.Slot);
			image2D.Image->Bind(image2D.Slot);
		}

		for (auto& cubeMap : m_CubeMap)
		{
			m_Shader->SetInt1(cubeMap.Name, cubeMap.Slot);
			cubeMap.Cubemap->Bind(cubeMap.Slot);
		}

		for (auto& storageBuffer : m_StorageBuffer)
		{
			storageBuffer.Buffer->Bind(storageBuffer.Slot);
		}
		RenderCommand::SetCull(m_CullMode);
		RenderCommand::SetBlend(m_BlendMode, m_BlendSourceFactor, m_BlendDestinationFactor);
		RenderCommand::EnableDepthTest(m_EnableDepthTest);
		RenderCommand::EnableDepthWrite(m_EnableDepthWrite);
	}
	void OpenGLMaterial::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
		m_Shader->Bind();
		m_Shader->SetIntArray(name, value, count);
	}
	void OpenGLMaterial::SetShader(const Ref<Shader>& shader)
	{
		GE_CORE_CRITICAL("暂时还没有写材质的Shader更换");
	}
	void OpenGLMaterial::SetMatrix4x4(const std::string& name, const Matrix4x4& value)
	{
		
	}

}
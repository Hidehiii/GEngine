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
			if (size > 0)
			{
				m_UniformBuffer = std::dynamic_pointer_cast<OpenGLUniformBuffer>(UniformBuffer::Create(size, 0));
			}
			// Read blend type and factor
			m_BlendModeColor				= m_Shader->GetBlendColor();
			m_BlendModeAlpha				= m_Shader->GetBlendAlpha();
			m_BlendColorSourceFactor		= m_Shader->GetBlendColorSrc();
			m_BlendAlphaSourceFactor		= m_Shader->GetBlendAlphaSrc();
			m_BlendColorDestinationFactor	= m_Shader->GetBlendColorDst();
			m_BlendAlphaDestinationFactor	= m_Shader->GetBlendAlphaDst();
			// cull mode
			m_CullMode					= m_Shader->GetCull();
			// Read depth test and depth mask
			m_EnableDepthWrite			= m_Shader->IsEnableDepthWrite();
			m_DepthTestOperation		= m_Shader->GetDepthTestOp();

			m_RenderStates				= m_Shader->GetRenderStates();

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
	void OpenGLMaterial::Update(CommandBuffer* cmdBuffer, const std::string& pass)
	{
		m_Shader->Use(pass);

		if (m_UniformBuffer)
			m_UniformBuffer->SetData(m_UniformsBuffer.ReadBytes(m_UniformsBuffer.GetSize()), m_UniformsBuffer.GetSize());

		for (auto& texture2D : m_Texture2D)
		{
			m_Shader->SetInt1(texture2D.Name, texture2D.Slot, pass);
			texture2D.Texture->Bind(cmdBuffer, texture2D.Slot);
		}

		for (auto& image2D : m_StorageImage2D)
		{
			m_Shader->SetInt1(image2D.Name, image2D.Slot, pass);
			image2D.Image->Bind(cmdBuffer, image2D.Slot);
		}

		for (auto& cubeMap : m_CubeMap)
		{
			m_Shader->SetInt1(cubeMap.Name, cubeMap.Slot, pass);
			cubeMap.Cubemap->Bind(cmdBuffer, cubeMap.Slot);
		}

		for (auto& storageBuffer : m_StorageBuffer)
		{
			storageBuffer.Buffer->Bind(storageBuffer.Slot);
		}

		Utils::SetCull(m_RenderStates[pass].Cull);
		Utils::SetBlend(m_RenderStates[pass].BlendColor, m_RenderStates[pass].BlendAlpha, 
			m_RenderStates[pass].BlendColorSrc, m_RenderStates[pass].BlendColorDst, 
			m_RenderStates[pass].BlendAlphaSrc, m_RenderStates[pass].BlendAlphaDst);
		Utils::SetDepthTest(m_RenderStates[pass].DepthTestOp);
		Utils::EnableDepthWrite(m_RenderStates[pass].DepthWrite);
	}
	void OpenGLMaterial::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
		
	}
	void OpenGLMaterial::SetShader(const Ref<Shader>& shader)
	{
		GE_CORE_CRITICAL("暂时还没有写材质的Shader更换");
	}

}
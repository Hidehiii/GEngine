#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Material.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "GEngine/Core/Buffer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace GEngine
{
	class GENGINE_API OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(const Ref<Shader>& shader, const std::string& name = "");
		virtual ~OpenGLMaterial() override;

		virtual void Update(CommandBuffer* cmdBuffer, const int& pass) override;

		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetShader(const Ref<Shader>& shader) override;

		virtual Buffer SetUniformBuffer(const int& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf) override;
		
	private:
		Ref<OpenGLShader>														m_Shader;
		std::vector<std::unordered_map<uint32_t, Ref<OpenGLUniformBuffer>>>		m_UniformBuffers; // pass { bind point : buffer }
	};
}



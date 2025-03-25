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

		virtual void Update(CommandBuffer* cmdBuffer) override;

		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;

		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetShader(const Ref<Shader>& shader) override;
		
	private:
		Ref<OpenGLShader>									m_Shader;
		Ref<OpenGLUniformBuffer>							m_UniformBuffer;
	};
}



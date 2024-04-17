#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace GEngine
{
	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLShader>(path);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	void ShaderLibrary::Add(Ref<Shader>& shader)
	{
		auto& name = shader->GetShaderName();
		GE_CORE_ASSERT(Exists(name) == false, "Shader already exists!");
		m_Shaders[name] = shader;
	}
	void ShaderLibrary::Add(const std::string& name, Ref<Shader>& shader)
	{
		shader->SetShaderName(name);
		GE_CORE_ASSERT(Exists(name) == false, "Shader already exists!");
		m_Shaders[name] = shader;
	}
	Ref<Shader> ShaderLibrary::Load(const std::string& path)
	{
		auto shader = Shader::Create(path);
		Add(shader);
		return shader;
	}
	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		GE_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}
}
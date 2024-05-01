#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace GEngine
{
	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::m_Shaders = std::unordered_map<std::string, Ref<Shader>>();

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			Ref<Shader> shader = CreateRef<OpenGLShader>(path);
			return shader;
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
			Ref<Shader> shader = CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
			return shader;
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	std::string ShaderLibrary::Add(Ref<Shader>& shader)
	{
		auto& name = shader->GetShaderName();
		GE_CORE_ASSERT(Exists(name) == false, "Shader already exists!");
		m_Shaders[name] = shader;
		return name;
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
	std::vector<std::string> ShaderLibrary::GetShaderNames()
	{
		std::vector<std::string> names;
		for (auto& pair : m_Shaders)
		{
			names.push_back(pair.first);
		}
		return names;
	}
}
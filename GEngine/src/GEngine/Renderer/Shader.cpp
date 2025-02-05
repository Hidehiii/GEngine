#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace GEngine
{
	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::m_Shaders = std::unordered_map<std::string, Ref<Shader>>();

	std::string     const	ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP = "GE_ATTACHMENT_UV_STARTS_AT_TOP";
	std::string		const	ShaderMacroName::GE_BINDING_START				= "GE_BINDING_START";

	std::string		const	ShaderDataFlag::None							= "#None";
	std::string		const	ShaderDataFlag::Name							= "#Name";
	std::string		const	ShaderDataFlag::Blend							= "#Blend";
	std::string		const	ShaderDataFlag::DepthMask						= "#DepthMask";
	std::string		const	ShaderDataFlag::DepthTest						= "#DepthTest";
	std::string		const	ShaderDataFlag::Properties						= "#Properties";
	std::string		const	ShaderDataFlag::Type							= "#Type";
					
	std::string     const	ShaderStage::Vertex								= "vertex";
	std::string     const	ShaderStage::Fragment							= "fragment";
	std::string     const	ShaderStage::Pixel								= "pixel";
	std::string		const	ShaderStage::Compute							= "compute";
	std::string		const	ShaderStage::TessellationControl				= "tessellationControl";
	std::string		const	ShaderStage::TessellationEvaluation				= "tessellationEvaluation";
	std::string		const	ShaderStage::Geometry							= "geometry";

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
		case RendererAPI::API::Vulkan: {
			Ref<Shader> shader = CreateRef<VulkanShader>(path);
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
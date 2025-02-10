#include "GEpch.h"
#include "VulkanShader.h"
#include "GEngine/Renderer/Material.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include <shaderc/shaderc.hpp>
#include <SPIRVCross/spirv_cross.hpp>
#include <SPIRVCross/spirv_glsl.hpp>

class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
{
	shaderc_include_result* GetInclude(
		const char* requested_source,
		shaderc_include_type type,
		const char* requesting_source,
		size_t include_depth) override
	{
		const std::string name = std::string(requested_source);
		const std::string contents = GEngine::Utils::ReadFile(name);

		auto container = new std::array<std::string, 2>;
		(*container)[0] = name;
		(*container)[1] = contents;

		auto data = new shaderc_include_result;

		data->user_data = container;

		data->source_name = (*container)[0].data();
		data->source_name_length = (*container)[0].size();

		data->content = (*container)[1].data();
		data->content_length = (*container)[1].size();

		return data;
	};

	void ReleaseInclude(shaderc_include_result* data) override
	{
		delete static_cast<std::array<std::string, 2>*>(data->user_data);
		delete data;
	};
};

namespace GEngine
{
	namespace Utils
	{

		static std::string ShaderTypeFromString(const std::string& type)
		{
			if (ToLower(type) == ShaderStage::Vertex)
				return ShaderStage::Vertex;
			if (ToLower(type) == ShaderStage::Fragment || ToLower(type) == ShaderStage::Pixel)
				return ShaderStage::Fragment;
			if (ToLower(type) == ShaderStage::Compute)
				return ShaderStage::Compute;
			if (ToLower(type) == ShaderStage::TessellationControl)
				return ShaderStage::TessellationControl;
			if (ToLower(type) == ShaderStage::TessellationEvaluation)
				return ShaderStage::TessellationEvaluation;
			if (ToLower(type) == ShaderStage::Geometry)
				return ShaderStage::Geometry;

			GE_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}
		static const char* GLShaderStageCachedOpenGLFileExtension(std::string stage)
		{
			if (stage == ShaderStage::Vertex)					return ".cached_opengl.vert";
			if (stage == ShaderStage::Fragment)					return ".cached_opengl.frag";
			if (stage == ShaderStage::Compute)					return ".cached_opengl.comp";
			if (stage == ShaderStage::TessellationControl)		return ".cached_opengl.tesc";
			if (stage == ShaderStage::TessellationEvaluation)	return ".cached_opengl.tese";
			if (stage == ShaderStage::Geometry)					return ".cached_opengl.geom";
			GE_CORE_ASSERT(false, "");
			return "";
		}
		static const char* GLShaderStageCachedVulkanFileExtension(std::string stage)
		{
			if (stage == ShaderStage::Vertex)					return ".cached_vulkan.vert";
			if (stage == ShaderStage::Fragment)					return ".cached_vulkan.frag";
			if (stage == ShaderStage::Compute)					return ".cached_vulkan.comp";
			if (stage == ShaderStage::TessellationControl)		return ".cached_vulkan.tesc";
			if (stage == ShaderStage::TessellationEvaluation)	return ".cached_vulkan.tese";
			if (stage == ShaderStage::Geometry)					return ".cached_vulkan.geom";
			GE_CORE_ASSERT(false, "");
			return "";
		}
		static shaderc_shader_kind ShaderStageToShaderC(std::string stage)
		{
			if (stage == ShaderStage::Vertex)					return shaderc_glsl_vertex_shader;
			if (stage == ShaderStage::Fragment)					return shaderc_glsl_fragment_shader;
			if (stage == ShaderStage::Compute)					return shaderc_glsl_compute_shader;
			if (stage == ShaderStage::TessellationControl)		return shaderc_glsl_tess_control_shader;
			if (stage == ShaderStage::TessellationEvaluation)	return shaderc_glsl_tess_evaluation_shader;
			if (stage == ShaderStage::Geometry)					return shaderc_glsl_geometry_shader;
			GE_CORE_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}
	}

	VulkanShader::VulkanShader(const std::string& path)
	{
		m_FilePath = path;
		Utils::CreateCacheDirectoryIfNeeded();
		std::string src		= Utils::ReadFile(path);
		auto shaderSources	= PreProcess(src);
		CompileOrGetVulkanBinaries(shaderSources);
	}
	VulkanShader::~VulkanShader()
	{
	}
	void VulkanShader::CreateShaderModule()
	{
		if (m_VulkanSPIRV.find(ShaderStage::Vertex) != m_VulkanSPIRV.end())
			m_VertexShaderModule	= CreateShaderModule(m_VulkanSPIRV[ShaderStage::Vertex]);
		if (m_VulkanSPIRV.find(ShaderStage::Fragment) != m_VulkanSPIRV.end())
			m_FragmentShaderModule	= CreateShaderModule(m_VulkanSPIRV[ShaderStage::Fragment]);
		if (m_VulkanSPIRV.find(ShaderStage::Compute) != m_VulkanSPIRV.end())
			m_ComputeShaderModule	= CreateShaderModule(m_VulkanSPIRV[ShaderStage::Compute]);
		if (m_VulkanSPIRV.find(ShaderStage::TessellationControl) != m_VulkanSPIRV.end())
			m_TessellationControlShaderModule = CreateShaderModule(m_VulkanSPIRV[ShaderStage::TessellationControl]);
		if (m_VulkanSPIRV.find(ShaderStage::TessellationEvaluation) != m_VulkanSPIRV.end())
			m_TessellationEvaluationShaderModule = CreateShaderModule(m_VulkanSPIRV[ShaderStage::TessellationEvaluation]);
		if (m_VulkanSPIRV.find(ShaderStage::Geometry) != m_VulkanSPIRV.end())
			m_GeometryShaderModule = CreateShaderModule(m_VulkanSPIRV[ShaderStage::Geometry]);
	}
	void VulkanShader::DestroyShaderModule()
	{
		if (m_VulkanSPIRV.find(ShaderStage::Vertex) != m_VulkanSPIRV.end())
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_VertexShaderModule, nullptr);
		if (m_VulkanSPIRV.find(ShaderStage::Fragment) != m_VulkanSPIRV.end())
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_FragmentShaderModule, nullptr);
		if (m_VulkanSPIRV.find(ShaderStage::Compute) != m_VulkanSPIRV.end())
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_ComputeShaderModule, nullptr);
		if (m_VulkanSPIRV.find(ShaderStage::TessellationControl) != m_VulkanSPIRV.end())
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_TessellationControlShaderModule, nullptr);
		if (m_VulkanSPIRV.find(ShaderStage::TessellationEvaluation) != m_VulkanSPIRV.end())
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_TessellationEvaluationShaderModule, nullptr);
		if (m_VulkanSPIRV.find(ShaderStage::Geometry) != m_VulkanSPIRV.end())
			vkDestroyShaderModule(VulkanContext::Get()->GetDevice(), m_GeometryShaderModule, nullptr);

		m_VertexShaderModule					= nullptr;
		m_FragmentShaderModule					= nullptr;
		m_ComputeShaderModule					= nullptr;
		m_TessellationControlShaderModule		= nullptr;
		m_TessellationEvaluationShaderModule	= nullptr;
		m_GeometryShaderModule					= nullptr;
	}
	VkShaderModule VulkanShader::GetShaderModule(std::string stage)
	{
		if (stage == ShaderStage::Pixel || stage == ShaderStage::Fragment)
			return m_FragmentShaderModule;
		else if (stage == ShaderStage::Vertex)
			return m_VertexShaderModule;
		else if (stage == ShaderStage::Compute)
			return m_ComputeShaderModule;
		else if (stage == ShaderStage::TessellationControl)
			return m_TessellationControlShaderModule;
		else if (stage == ShaderStage::TessellationEvaluation)
			return m_TessellationEvaluationShaderModule;
		else if (stage == ShaderStage::Geometry)
			return m_GeometryShaderModule;

		GE_CORE_ASSERT(false, "Unknown shader stage");
		return nullptr;
	}
	void VulkanShader::SetMacroBool(std::string& source)
	{
		for (int i = 0; i < m_MacroBools.size(); i++)
		{
			Utils::SetShaderMacroBool(source, m_MacroBools[i].first, m_MacroBools[i].second);
		}
	}
	void VulkanShader::SetMacroExp(std::string& source)
	{
		for (int i = 0; i < m_MacroExps.size(); i++)
		{
			Utils::SetShaderMacroExpression(source, m_MacroExps[i].first, m_MacroExps[i].second);
		}
	}
	VkShaderModule VulkanShader::CreateShaderModule(const std::vector<uint32_t>& code)
	{
		VkShaderModuleCreateInfo	createInfo{};
		createInfo.sType			= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize			= code.size() * sizeof(uint32_t);
		createInfo.pCode			= code.data();
		VkShaderModule				shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::Get()->GetDevice(), &createInfo, nullptr, &shaderModule));
		return shaderModule;
	}
	std::unordered_map<std::string, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		std::unordered_map<std::string, std::string> shaderSources;

		const char* typeToken			= "#Type";
		size_t typeTokenLength			= strlen(typeToken);

		// find Name 
		m_Name = Utils::ProcessShaderName(source);

		// find Blend
		Utils::ProcessShaderBlend(source, m_BlendModeColor, m_BlendModeAlpha, m_BlendColorSourceFactor, m_BlendColorDestinationFactor, m_BlendAlphaSourceFactor, m_BlendAlphaDestinationFactor);


		// find DepthWrite
		Utils::ProcessShaderDepthWrite(source, m_EnableDepthWrite);

		// find DepthTest
		Utils::ProcessShaderDepthTest(source, m_DepthTestOperation);

		// find Cull
		Utils::ProcessShaderCull(source, m_CullMode);

		// find Properties
		Utils::ProcessShaderProperties(source, m_UniformCache, m_Texture2DCache, m_CubeMapCache, m_StorageImage2DCache, m_StorageBufferCache, 0, Shader::s_SlotOffset, 0);

		// find Type

		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = type.find(' ', index)) != std::string::npos)
			{
				type.erase(index, 1);
			}
			GE_CORE_ASSERT(Utils::ShaderTypeFromString(type).empty() == false, "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}
		return shaderSources;
	}
	void VulkanShader::CompileOrGetVulkanBinaries(std::unordered_map<std::string, std::string>& shaderSources)
	{
		shaderc::Compiler			compiler;
		shaderc::CompileOptions		options;
		options.SetIncluder(std::make_unique<ShaderIncluder>());
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		//options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory		= Utils::GetCacheDirectory();

		auto& shaderData			= m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath	= m_FilePath;
			std::filesystem::path cachedPath		= cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size		= in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data		= shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				Preprocess(source);
				shaderc::SpvCompilationResult module		= compiler.CompileGlslToSpv(source, Utils::ShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					GE_CORE_ERROR("Error shader context:\n{}", source);
					GE_CORE_ASSERT(false, module.GetErrorMessage());
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}
	void VulkanShader::Reflect(const std::string stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler			compiler(shaderData);
		spirv_cross::ShaderResources	resources			= compiler.get_shader_resources();

		GE_CORE_INFO("VulkanShader::Reflect - {0} {1}", stage, m_FilePath);
		GE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GE_CORE_TRACE("    {0} sampled images", resources.sampled_images.size());
		GE_CORE_TRACE("    {0} separate images", resources.separate_images.size());
		GE_CORE_TRACE("    {0} separate samplers", resources.separate_samplers.size());

		GE_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType							= compiler.get_type(resource.base_type_id);
			uint32_t bufferSize								= compiler.get_declared_struct_size(bufferType);
			uint32_t binding								= compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount									= bufferType.member_types.size();

			GE_CORE_TRACE("  {0}", resource.name);
			GE_CORE_TRACE("    Size = {0}", bufferSize);
			GE_CORE_TRACE("    Binding = {0}", binding);
			GE_CORE_TRACE("    Members = {0}", memberCount);
		}
	}
}
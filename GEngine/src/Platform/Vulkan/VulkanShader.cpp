#include "GEpch.h"
#include "VulkanShader.h"
#include "GEngine/Graphics/Material.h"
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
		const char* GLShaderStageCachedVulkanFileExtension(std::string stage)
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
		shaderc_shader_kind ShaderStageToShaderC(std::string stage)
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
		m_FilePath			= path;
		Utils::CreateCacheDirectoryIfNeeded();
		std::string src		= Utils::ReadFile(path);
		std::vector<std::vector<std::string>> shaderStages;
		std::vector<std::string> shaderSrcCode;
		Preprocess(src, shaderStages, shaderSrcCode);
		Compile(shaderStages, shaderSrcCode);
	}
	VulkanShader::~VulkanShader()
	{
	}
	bool VulkanShader::Compile(std::vector<std::vector<std::string>>& passStages, std::vector<std::string>& shaderSrcCodes)
	{
		GE_CORE_ASSERT(passStages.size() == shaderSrcCodes.size(), "pass stages and shader src codes size not equal");
		for (int i = 0; i < passStages.size(); ++i)
		{
			for (int j = 0; j < passStages[i].size(); ++j)
			{
				std::vector<LPWSTR> args;
				std::vector<uint8_t> output;
				std::vector<uint8_t> reflection;
				ShaderCompiler::Get()->Compile(shaderSrcCodes.at(i), args, output, reflection);
			}
		}
		return true;
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
	void VulkanShader::ProcessShaderSource(const std::string& source)
	{
		m_Name = Utils::ProcessShaderName(source);
		Utils::ProcessShaderProperties(source, m_UniformCache, m_Texture2DCache, m_CubeMapCache, m_StorageImage2DCache, m_StorageBufferCache, 0, s_SlotOffset, 0);
		// multi pass
		Utils::ProcessShaderBlocks(source, m_ShaderBlocks);
		Utils::ProcessShaderPasses(source, m_ShaderBlocks, m_ShaderPasses, m_RenderStates);
		
	}
	std::unordered_map<std::string, std::vector<uint32_t>> VulkanShader::CompileVulkanBinaries(std::pair<std::string, ShaderPass> pass)
	{
		shaderc::Compiler			compiler;
		shaderc::CompileOptions		options;
		options.SetIncluder(std::make_unique<ShaderIncluder>());
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		std::unordered_map<std::string, std::vector<uint32_t>> shaderData;
		for (auto&& [stage, source] : pass.second.Stages)
		{
			std::filesystem::path shaderFilePath	= m_FilePath;
			std::filesystem::path cachedPath		= cacheDirectory / (shaderFilePath.filename().string() + "." + pass.first + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				Preprocess(source);
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::ShaderStageToShaderC(stage), m_FilePath.c_str(), options);
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

		return shaderData;
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
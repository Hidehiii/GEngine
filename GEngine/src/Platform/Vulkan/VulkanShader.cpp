#include "GEpch.h"
#include "VulkanShader.h"
#include "GEngine/Renderer/Material.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture2D.h"


#include <shaderc/shaderc.hpp>
#include <SPIRVCross/spirv_cross.hpp>
#include <SPIRVCross/spirv_glsl.hpp>

namespace GEngine
{
	namespace Utils
	{
		static uint32_t ShaderBlendFactorFromString(const std::string& factor)
		{
			if (ToUpper(factor) == "SRCALPHA")			return (uint32_t)VK_BLEND_FACTOR_SRC_ALPHA;
			if (ToUpper(factor) == "DSTALPHA")			return (uint32_t)VK_BLEND_FACTOR_DST_ALPHA;
			if (ToUpper(factor) == "SRCCOLOR")			return (uint32_t)VK_BLEND_FACTOR_SRC_COLOR;
			if (ToUpper(factor) == "DSTCOLOR")			return (uint32_t)VK_BLEND_FACTOR_DST_COLOR;
			if (ToUpper(factor) == "ONEMINUSSRCALPHA")	return (uint32_t)VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			if (ToUpper(factor) == "ONEMINUSDSTALPHA")	return (uint32_t)VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			if (ToUpper(factor) == "ONEMINUSSRCCOLOR")	return (uint32_t)VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			if (ToUpper(factor) == "ONEMINUSDSTCOLOR")	return (uint32_t)VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			if (ToUpper(factor) == "ONE")				return (uint32_t)VK_BLEND_FACTOR_ONE;
			if (ToUpper(factor) == "ZERO")				return (uint32_t)VK_BLEND_FACTOR_ZERO;

			GE_CORE_ASSERT(false, "Unknown blend factor! " + factor);
		}
		static MaterialBlendMode ShaderBlendTypeFromString(const std::string& type)
		{
			if (ToLower(type) == "none")			return MaterialBlendMode::None;
			if (ToLower(type) == "alpha")			return MaterialBlendMode::Alpha;
			if (ToLower(type) == "additive")		return MaterialBlendMode::Additive;
			if (ToLower(type) == "multiply")		return MaterialBlendMode::Multiply;
			if (ToLower(type) == "customized")		return MaterialBlendMode::Customized;

			GE_CORE_ASSERT(false, "Unknown blend type! " + type);
			return MaterialBlendMode::None;
		}
		static std::string ShaderTypeFromString(const std::string& type)
		{
			if (ToLower(type) == ShaderStage::Vertex)
				return ShaderStage::Vertex;
			if (ToLower(type) == ShaderStage::Fragment || ToLower(type) == ShaderStage::Pixel)
				return ShaderStage::Fragment;

			GE_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}
		static const char* GLShaderStageCachedOpenGLFileExtension(std::string stage)
		{
			if (stage == ShaderStage::Vertex)			return ".cached_opengl.vert";
			else if (stage == ShaderStage::Fragment)	return ".cached_opengl.frag";
			GE_CORE_ASSERT(false, "");
			return "";
		}
		static const char* GLShaderStageCachedVulkanFileExtension(std::string stage)
		{
			if (stage == ShaderStage::Vertex)		return ".cached_vulkan.vert";
			else if(stage == ShaderStage::Fragment) return ".cached_vulkan.frag";
			GE_CORE_ASSERT(false, "");
			return "";
		}
		static shaderc_shader_kind ShaderStageToShaderC(std::string stage)
		{
			if (stage == ShaderStage::Vertex)			return shaderc_glsl_vertex_shader;
			else if (stage == ShaderStage::Fragment)	return shaderc_glsl_fragment_shader;
			GE_CORE_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}
	}

	VulkanShader::VulkanShader(const std::string& path)
		: m_FilePath(path)
	{
		GE_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();
		std::string src		= ReadFile(path);
		auto shaderSources	= PreProcess(src);
		CompileOrGetVulkanBinaries(shaderSources);
		//CompileOrGetOpenGLBinaries(shaderSources);
	}
	VulkanShader::VulkanShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
	}
	VulkanShader::~VulkanShader()
	{
	}
	void VulkanShader::Bind() const
	{
		
	}
	void VulkanShader::SetInt1(const std::string& name, int value)
	{
	}
	void VulkanShader::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
	}
	void VulkanShader::SetFloat1(const std::string& name, float value)
	{
	}
	void VulkanShader::SetFloat2(const std::string& name, const Vector2& value)
	{
	}
	void VulkanShader::SetFloat3(const std::string& name, const Vector3& value)
	{
	}
	void VulkanShader::SetFloat4(const std::string& name, const Vector4& value)
	{
	}
	void VulkanShader::SetMat4x4(const std::string& name, const Matrix4x4& value)
	{
	}
	void VulkanShader::SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count)
	{
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
	std::string VulkanShader::ReadFile(const std::string& path)
	{
		std::string		src;
		std::ifstream	file(path, std::ios::in | std::ios::binary);

		GE_CORE_ASSERT(file, "Shader file " + path + " not found");

		file.seekg(0, std::ios::end);
		src.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&src[0], src.size());
		file.close();
		return src;
	}
	std::unordered_map<std::string, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		std::unordered_map<std::string, std::string> shaderSources;

		const char* nameToken			= "#Name";
		size_t nameTokenLength			= strlen(nameToken);

		const char* blendToken			= "#Blend";
		size_t blendTokenLength			= strlen(blendToken);

		const char* depthWriteToken		= "#DepthWrite";
		size_t depthWriteTokenLength	= strlen(depthWriteToken);

		const char* depthTestToken		= "#DepthTest";
		size_t depthTestTokenLength		= strlen(depthTestToken);

		const char* propertyToken		= "#Properties";
		size_t propertyTokenLength		= strlen(propertyToken);

		const char* typeToken			= "#Type";
		size_t typeTokenLength			= strlen(typeToken);

		// find Name 

		size_t pos				= source.find(nameToken, 0);
		if (pos != std::string::npos)
		{
			size_t eol			= source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin		= pos + nameTokenLength + 1;
			std::string name	= source.substr(begin, eol - begin);
			int index			= 0;
			while ((index = name.find(' ', index)) != std::string::npos)
			{
				name.erase(index, 1);
			}
			m_Name				= name;
			GE_CORE_TRACE("Shader name: {0}", m_Name);
		}

		// find Blend
		pos								= source.find(blendToken, 0);
		if (pos != std::string::npos)
		{
			size_t eol					= source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin				= pos + blendTokenLength + 1;
			std::string blendString = source.substr(begin, eol - begin);
			blendString					= Utils::RemoveCharFromString(blendString, ';');
			blendString					= Utils::RemoveCharFromString(blendString, '\r');
			blendString					= Utils::RemoveCharFromString(blendString, '\n');
			std::vector<std::string>	blends = Utils::SplitString(blendString, ' ');
			GE_CORE_ASSERT(blends.size() == 3 || blends.size() == 1, "Syntax error");
			m_BlendType					= (int)Utils::ShaderBlendTypeFromString(blends.at(0));
			if (blends.size() == 3)
			{
				m_BlendSourceFactor		= Utils::ShaderBlendFactorFromString(blends.at(1));
				m_BlendDestinationFactor = Utils::ShaderBlendFactorFromString(blends.at(2));
				GE_CORE_TRACE("Blend type: {0}, Src factor: {1}, Dst factor: {2}", blends.at(0), blends.at(1), blends.at(2));
			}
			else
			{
				GE_CORE_TRACE("Blend type: {0}", blends.at(0));
			}
		}

		// find DepthWrite
		pos					= source.find(depthWriteToken, 0);
		if (pos != std::string::npos)
		{
			size_t eol		= source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin	= pos + depthWriteTokenLength + 1;
			std::string		depthMaskProp = source.substr(begin, eol - begin);
			int index		= 0;
			while ((index = depthMaskProp.find(' ', index)) != std::string::npos)
			{
				depthMaskProp.erase(index, 1);
			}
			m_EnableDepthWrite = Utils::ShaderBoolFromString(depthMaskProp);
			GE_CORE_TRACE("DepthMask: {0}", m_EnableDepthWrite);
		}

		// find DepthTest
		pos					= source.find(depthTestToken, 0);
		if (pos != std::string::npos)
		{
			size_t eol		= source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin	= pos + depthTestTokenLength + 1;
			std::string		depthTestProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = depthTestProp.find(' ', index)) != std::string::npos)
			{
				depthTestProp.erase(index, 1);
			}
			m_EnableDepthTest = Utils::ShaderBoolFromString(depthTestProp);
			GE_CORE_TRACE("DepthTest: {0}", m_EnableDepthTest);
		}

		// find Properties
		pos						= source.find(propertyToken, 0);
		if (pos != std::string::npos)
		{
			// split properties by \n
			size_t eol			= source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t nextLinePos	= source.find_first_not_of("\r\n", eol);
			size_t end			= source.find(typeToken, pos);
			std::string properties = source.substr(nextLinePos, end - nextLinePos);
			std::vector<std::string> props = Utils::SplitString(properties, '\n');
			for (auto& prop : props)
			{
				//prop = Utils::RemoveCharFromString(prop, ' ');
				prop = Utils::RemoveCharFromString(prop, '\r');
				prop = Utils::RemoveCharFromString(prop, '\n');
				prop = Utils::RemoveCharFromString(prop, '\t');
				prop = Utils::RemoveCharFromString(prop, ';');
			}
			for (auto it = props.begin(); it != props.end();)
			{
				if (it->empty())
					it = props.erase(it);
				else
					++it;
			}
			uint32_t location = 0;
			uint32_t slot = VulkanShader::s_Slot;
			// split properties by " "
			for (auto& prop : props)
			{
				std::vector<std::string> propData = Utils::SplitString(prop, ' ');
				for (auto& prop : propData)
				{
					prop = Utils::RemoveCharFromString(prop, ' ');
				}
				for (auto it = propData.begin(); it != propData.end();)
				{
					if (it->empty())
						it = propData.erase(it);
					else
						++it;
				}
				GE_CORE_ASSERT(propData.size() == 2, "Invalid property syntax");
				std::string propName = propData[1];
				std::string propType = propData[0];
				if (Utils::ShaderUniformTypeFromString(propType) != ShaderUniformType::None &&
					Utils::ShaderUniformTypeFromString(propType) != ShaderUniformType::Sampler2D &&
					Utils::ShaderUniformTypeFromString(propType) != ShaderUniformType::SamplerCube && 
					Utils::ShaderUniformTypeFromString(propType) != ShaderUniformType::StorageImage2D &&
					Utils::ShaderUniformTypeFromString(propType) != ShaderUniformType::StorageBuffer)
				{
					ShaderUniform			uniform;
					uniform.Name			= propName;
					uniform.Type			= Utils::ShaderUniformTypeFromString(propType);
					uniform.Size			= ShaderUniformTypeSize(uniform.Type);
					uniform.Location		= location;
					location				+= uniform.Size;
					m_UniformCache.push_back(uniform);
					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property Size: {2}, Property Location: {3}", uniform.Name, propType, uniform.Size, uniform.Location);
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::Sampler2D)
				{
					ShaderUniformTexture2D	uniform;
					uniform.Name			= propName;
					uniform.Slot			= slot;
					uniform.Texture			= Texture2D::White();
					
					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property Location: {2}", uniform.Name, propType, slot);

					slot++;
					m_Texture2DCache.push_back(uniform);
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::SamplerCube)
				{
					GE_CORE_ASSERT(false, "Not implemented yet");
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::StorageImage2D)
				{
					ShaderUniformStorageImage2D	uniform;
					uniform.Name				= propName;
					uniform.Slot				= slot;
					uniform.Image				= nullptr;

					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property Location: {2}", uniform.Name, propType, slot);

					slot++;
					m_StorageImage2DCache.push_back(uniform);
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::StorageBuffer)
				{
					GE_CORE_ASSERT(false, "Not implemented yet");
				}
			}
		}



		// find Type

		pos = source.find(typeToken, 0);

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
	void VulkanShader::Compile(std::unordered_map<std::string, std::string>& source)
	{
	}
	void VulkanShader::CompileOrGetVulkanBinaries(std::unordered_map<std::string, std::string>& shaderSources)
	{
		shaderc::Compiler			compiler;
		shaderc::CompileOptions		options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize			= false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

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
				SetMacroBool(source);
				shaderc::SpvCompilationResult module		= compiler.CompileGlslToSpv(source, Utils::ShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
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
	void VulkanShader::CompileOrGetOpenGLBinaries(const std::unordered_map<std::string, std::string>& shaderSources)
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

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

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::ShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
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
	}
	void VulkanShader::Reflect(const std::string stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler			compiler(shaderData);
		spirv_cross::ShaderResources	resources			= compiler.get_shader_resources();

		GE_CORE_TRACE("VulkanShader::Reflect - {0} {1}", stage, m_FilePath);
		GE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GE_CORE_TRACE("    {0} resources", resources.sampled_images.size());

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
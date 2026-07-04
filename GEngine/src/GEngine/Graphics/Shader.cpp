#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Tools/StringHelper.h"
#include "GEngine/Tools/FileSystemHelper.h"
#include "GEngine/Tools/Serializer.h"
#include "GEngine/Tools/OpenSSLTool.h"
#include "GEngine/Tools/ShaderCompiler.h"
#include "GEngine/Application.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/D3D12/D3D12Shader.h"

namespace GEngine
{
	std::unordered_map<std::string, Ref<Shader>> Shader::s_Shaders = std::unordered_map<std::string, Ref<Shader>>();

	std::string     const	ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP = VAR_NAME(GE_ATTACHMENT_UV_STARTS_AT_TOP);
	std::string		const	ShaderMacroName::GE_BINDING_START				= VAR_NAME(GE_BINDING_START);
	std::string		const	ShaderMacroName::GE_MATERIAL_DEFINE				= VAR_NAME(GE_MATERIAL_DEFINE);
	std::string		const	ShaderMacroName::GE_GRAPHICS_API_OPENGL			= VAR_NAME(GE_GRAPHICS_API_OPENGL);
	std::string		const	ShaderMacroName::GE_GRAPHICS_API_VULKAN			= VAR_NAME(GE_GRAPHICS_API_VULKAN);
	std::string		const	ShaderMacroName::GE_GRAPHICS_API_D3D12			= VAR_NAME(GE_GRAPHICS_API_D3D12);
	std::string		const	ShaderMacroName::GE_GRAPHICS_API				= VAR_NAME(GE_GRAPHICS_API);

	namespace Utils
	{
		uint32_t GetShaderPropertyTypeSize(ShaderPropertyType type)
		{
			switch (type)
			{
			// scalar size
			case SHADER_PROPERTY_TYPE_INT:			return 4;
			case SHADER_PROPERTY_TYPE_FLOAT:		return 4;
			case SHADER_PROPERTY_TYPE_VECTOR_2:		return 4 * 2;
			case SHADER_PROPERTY_TYPE_VECTOR_3:		return 4 * 3;
			case SHADER_PROPERTY_TYPE_VECTOR_4:		return 4 * 4;
			case SHADER_PROPERTY_TYPE_MATRIX_2X2:	return 4 * 2 * 2;
			case SHADER_PROPERTY_TYPE_MATRIX_2X3:	return 4 * 2 * 3;
			case SHADER_PROPERTY_TYPE_MATRIX_2X4:	return 4 * 2 * 4;
			case SHADER_PROPERTY_TYPE_MATRIX_3X2:	return 4 * 3 * 2;
			case SHADER_PROPERTY_TYPE_MATRIX_3X3:	return 4 * 3 * 3;
			case SHADER_PROPERTY_TYPE_MATRIX_3X4:	return 4 * 3 * 4;
			case SHADER_PROPERTY_TYPE_MATRIX_4X2:	return 4 * 4 * 2;
			case SHADER_PROPERTY_TYPE_MATRIX_4X3:	return 4 * 4 * 3;
			case SHADER_PROPERTY_TYPE_MATRIX_4X4:	return 4 * 4 * 4;


			// ptr size
			case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D:
			case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_CUBE:
			case SHADER_PROPERTY_TYPE_SAMPLER_TEXTURE_2D_ARRAY:
			case SHADER_PROPERTY_TYPE_SAMPLER:
			case SHADER_PROPERTY_TYPE_TEXTURE_2D:
			case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
			case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY:
			case SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D:
			case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:
#ifdef GE_ARCH_X64
				return 8;
#else
#error "Shader uniform pointer size is not defined for this architecture!"
#endif
			}
			return 0;
		}

		CompareOperation ShaderCompareOperationFromString(const std::string& value)
		{
			if (StringHelper::ToLower(value) == VAR_NAME(less))			return COMPARE_OP_LESS;
			if (StringHelper::ToLower(value) == VAR_NAME(greater))		return COMPARE_OP_GREATER;
			if (StringHelper::ToLower(value) == VAR_NAME(lessequal))		return COMPARE_OP_LESS_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(lequal))			return COMPARE_OP_LESS_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(greaterequal))	return COMPARE_OP_GREATER_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(gequal))			return COMPARE_OP_GREATER_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(equal))			return COMPARE_OP_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(notequal))		return COMPARE_OP_NOT_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(always))			return COMPARE_OP_ALWAYS;

			return COMPARE_OP_LESS_EQUAL;
		}
		CullMode ShaderCullModeFromString(const std::string& value)
		{
			if (StringHelper::ToLower(value) == VAR_NAME(none))	return CULL_MODE_NONE;
			if (StringHelper::ToLower(value) == VAR_NAME(back))	return CULL_MODE_BACK;
			if (StringHelper::ToLower(value) == VAR_NAME(front))	return CULL_MODE_FRONT;
			return CULL_MODE_BACK;
		}
		
		BlendFactor ShaderBlendFactorFromString(const std::string& factor)
		{
			if (StringHelper::ToUpper(factor) == VAR_NAME(SRCALPHA))			return BLEND_FACTOR_SRC_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(DSTALPHA))			return BLEND_FACTOR_DST_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(SRCCOLOR))			return BLEND_FACTOR_SRC_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(DSTCOLOR))			return BLEND_FACTOR_DST_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSSRCALPHA))	return BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSDSTALPHA))	return BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSSRCCOLOR))	return BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSDSTCOLOR))	return BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONE))				return BLEND_FACTOR_ONE;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ZERO))				return BLEND_FACTOR_ZERO;

			GE_CORE_ASSERT(false, "Unknown blend factor! " + factor);
		}
		BlendMode ShaderBlendModeFromString(const std::string& type)
		{
			if (StringHelper::ToLower(type) == VAR_NAME(none))				return BLEND_MODE_NONE;
			if (StringHelper::ToLower(type) == VAR_NAME(add))					return BLEND_MODE_ADD;
			if (StringHelper::ToLower(type) == VAR_NAME(substract))			return BLEND_MODE_SUBSTRACT;
			if (StringHelper::ToLower(type) == VAR_NAME(reversesubstract))	return BLEND_MODE_REVERSE_SUBSTRACT;
			if (StringHelper::ToLower(type) == VAR_NAME(min))					return BLEND_MODE_MIN;
			if (StringHelper::ToLower(type) == VAR_NAME(max))					return BLEND_MODE_MAX;

			GE_CORE_ASSERT(false, "Unknown blend type! " + type);
			return BLEND_MODE_NONE;
		}
	}

	const ShaderPropertyType Shader::GetPropertyType(const std::string& name)
	{
		auto it = m_PropertyTypes.find(name);
		if (it != m_PropertyTypes.end())
			return it->second;
		return ShaderPropertyType();
	}

	void Shader::InitializeShader(const std::string& path, std::function<void(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>&)> processMachingCodeFunc)
	{
		m_FilePath = path;
		FileSystemHelper::CreateFolder(Application::Get().GetConfig()->GetShaderCacheDirectory());
		std::vector<std::string>												srcCodes; // each pass src code
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>	shaders; // pass { stage : code }
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>	reflectionMetas; // pass { stage : reflection meta }
		std::string																source = FileSystemHelper::ReadFileAsString(path);
		ShaderCacheInfo															cache;
		Preprocess(source, srcCodes);
		if (LoadFromCache(shaders, reflectionMetas, source))
		{
			ReflectShader(reflectionMetas);
			processMachingCodeFunc(shaders);
			GE_INFO("Load shader {} from cache.", m_Name);
			return;
		}
		Compile(srcCodes, shaders, reflectionMetas);
		ReflectShader(reflectionMetas);
		ComputeShaderCacheInfo(source, cache);
		SaveToCache(shaders, reflectionMetas, cache);
		processMachingCodeFunc(shaders);
	}

	bool Shader::IdentifyShaderCache(const std::string& source, ShaderCacheInfo& cache)
	{
		std::string hashCode = OpenSSLTool::ComputeMD5(source);
		return cache.Name == m_Name && cache.HashCode == hashCode;
	}

	void Shader::Preprocess(const std::string& source, std::vector<std::string>& shaderSrcCode)
	{
		std::stack<int> stack;
		std::string block;

		size_t pos = source.find("Shader", 0);
		pos = source.find("\"", pos);

		m_Name = source.substr(pos + 1, source.find("\"", pos + 1) - pos - 1);
		GE_CORE_INFO("Shader name: {}", m_Name);

		pos = source.find("{", pos);
		stack.push(1);


		while (source.find("Pass", pos) != std::string::npos)
		{
			GE_CORE_INFO("Shader pass {}:", m_PassReflections.size());

			m_PassReflections.emplace_back(ShaderReflectionInfo());
			pos = source.find("Pass", pos);
			pos = source.find("{", pos);
			stack.push(1);

			block = source.substr(pos + 1, source.find("Program", pos + 1) - pos - 1);

			std::string commandLine;
			size_t commandPos;
			// cull
			commandPos = block.find("Cull", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid cull command! " + commandLine);
				m_PassReflections.at(m_PassReflections.size() - 1).State.Cull = Utils::ShaderCullModeFromString(words[1]);
				GE_CORE_TRACE("	Cull mode: {}", words[1]);
			}
			//depth test
			commandPos = block.find("DepthTest", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid DepthTest command! " + commandLine);
				m_PassReflections.at(m_PassReflections.size() - 1).State.DepthTestOp = Utils::ShaderCompareOperationFromString(words[1]);
				GE_CORE_TRACE("	Depth test: {}", words[1]);
			}
			//depth write
			commandPos = block.find("DepthWrite", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid DepthWrite command! " + commandLine);
				m_PassReflections.at(m_PassReflections.size() - 1).State.DepthWrite = (StringHelper::ToLower(words[1]) == "true" || words[1] == "1");
				GE_CORE_TRACE("	Depth write: {}", words[1]);
			}
			//blend,用空格避免找到BlendOp等类似命令
			commandPos = block.find("Blend ", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 3 || words.size() == 5, "Invalid Blend command! " + commandLine);
				if (words.size() == 3)
				{
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendColorSrc = Utils::ShaderBlendFactorFromString(words[1]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendAlphaSrc = Utils::ShaderBlendFactorFromString(words[1]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendColorDst = Utils::ShaderBlendFactorFromString(words[2]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendAlphaDst = Utils::ShaderBlendFactorFromString(words[2]);
					GE_CORE_TRACE("	Blend mode: {} {}", words[1], words[2]);
				}
				if (words.size() == 5)
				{
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendColorSrc = Utils::ShaderBlendFactorFromString(words[1]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendAlphaSrc = Utils::ShaderBlendFactorFromString(words[3]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendColorDst = Utils::ShaderBlendFactorFromString(words[2]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendAlphaDst = Utils::ShaderBlendFactorFromString(words[4]);
					GE_CORE_TRACE("	Blend mode: {} {} {} {}", words[1], words[2], words[3], words[4]);
				}
			}
			// blend op
			commandPos = block.find("BlendOp", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2 || words.size() == 3, "Invalid BlendOp command! " + commandLine);
				if (words.size() == 2)
				{
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendColor = Utils::ShaderBlendModeFromString(words[1]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendAlpha = Utils::ShaderBlendModeFromString(words[1]);
					GE_CORE_TRACE("	Blend op: {}", words[1]);
				}
				if (words.size() == 3)
				{
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendColor = Utils::ShaderBlendModeFromString(words[1]);
					m_PassReflections.at(m_PassReflections.size() - 1).State.BlendAlpha = Utils::ShaderBlendModeFromString(words[2]);
					GE_CORE_TRACE("	Blend op: {} {}", words[1], words[2]);
				}
			}
			//color mask
			commandPos = block.find("ColorMask", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', ',', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				int mask = 0;
				for (size_t i = 1; i < words.size(); i++)
				{
					if (StringHelper::ToUpper(words[i]) == "R") mask |= COLOR_MASK_CHANNLE_R;
					if (StringHelper::ToUpper(words[i]) == "G") mask |= COLOR_MASK_CHANNLE_G;
					if (StringHelper::ToUpper(words[i]) == "B") mask |= COLOR_MASK_CHANNLE_B;
					if (StringHelper::ToUpper(words[i]) == "A") mask |= COLOR_MASK_CHANNLE_A;
				}
				m_PassReflections.at(m_PassReflections.size() - 1).State.ColorMask = mask;
				GE_CORE_TRACE("	Color mask: {}", mask);
			}
			//tag
			commandPos = block.find("Tag", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid Tag command! " + commandLine);
				m_PassReflections.at(m_PassReflections.size() - 1).State.Tag = words[1];
				GE_CORE_TRACE("	Tag: {}", words[1]);
			}
			//#pragma
			commandPos = 0;
			m_StageEntryPoints.push_back(std::unordered_map<std::string, std::string>());
			while (block.find("#pragma", commandPos) != std::string::npos)
			{
				commandPos = block.find("#pragma", commandPos);
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', ',', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 3, "Invalid pragma command! " + commandLine);
				if (StringHelper::ToUpper(words[1]) == "VERTEX")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_VERTEX] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "FRAGMENT")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_FRAGMENT] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "HULL")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_HULL] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "DOMAIN")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_DOMAIN] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "GEOMETRY")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_GEOMETRY] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "COMPUTE")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_COMPUTE] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "AMPLIFICATION")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_AMPLIFICATION] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "MESH")
					m_StageEntryPoints.at(m_StageEntryPoints.size() - 1)[SHADER_STAGE_MESH] = words[2];
				else
					GE_CORE_ASSERT(false, "Unknown shader stage in pragma command! " + commandLine);
				commandPos++;
			}
			


			pos = source.find("Program", pos);
			pos = source.find("{", pos);
			stack.push(1);
			size_t begin = pos + 1;
			pos++;
			while (pos < source.size())
			{
				if (source.at(pos) == '{')
				{
					stack.push(1);
				}
				if (source.at(pos) == '}')
				{
					stack.pop();
				}
				if (stack.size() == 2)
				{
					pos;
					break;
				}
				pos++;
			}
			block = source.substr(begin, pos - begin - 1);

			// src code
			shaderSrcCode.push_back(block);

			pos = source.find("}", pos + 1);
			stack.pop();
		}
		
	}

	bool Shader::Compile(const std::vector<std::string>& shaderSrcCodes, 
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders,
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionMetas)
	{
		GE_CORE_ASSERT(m_StageEntryPoints.size() == m_PassReflections.size(), "Size of pass and entryPoint not match!");
		for (int i = 0; i < m_StageEntryPoints.size(); i++)
		{
			GE_CORE_ASSERT(i < shaderSrcCodes.size(), "Shader source code size mismatch!");
			GE_CORE_ASSERT(m_StageEntryPoints.at(i).size() > 0, "No entry points found for shader pass " + std::to_string(i));
			shaders.push_back(std::unordered_map<std::string, std::vector<std::byte>>());
			reflectionMetas.push_back(std::unordered_map<std::string, std::vector<std::byte>>());
			
			for (auto&& [stage, entryPoint] : m_StageEntryPoints.at(i))
			{
				// 反射所有阶段资源的并集
				std::vector<std::byte> machineCode;
				std::vector<std::byte> reflectionMeta;
				bool result = ShaderCompiler::Get()->Compile(shaderSrcCodes.at(i), stage, entryPoint, machineCode, reflectionMeta);
				GE_CORE_ASSERT(result, "Failed to compile shader stage " + stage + " for pass " + std::to_string(i));
				shaders.at(i)[stage] = machineCode;
				reflectionMetas.at(i)[stage] = reflectionMeta;
			}
		}
		
		
		return true;
	}

	void Shader::ComputeShaderCacheInfo(const std::string& source, ShaderCacheInfo& cache)
	{
		cache.HashCode	= OpenSSLTool::ComputeMD5(source);
		cache.Name = m_Name;
	}

	void Shader::SaveToCache(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders, 
								const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionMetas, 
								ShaderCacheInfo& cache)
	{
		bool res = false;
		std::string graphicsAPIExt;
		std::string cacheFolder = Application::Get().GetConfig()->GetShaderCacheDirectory() + FileSystemHelper::GetDocumentNameWithoutExtension(m_FilePath);
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL: graphicsAPIExt = GRAPHICS_API_EXT_OPENGL; break;
		case GRAPHICS_API_VULKAN: graphicsAPIExt = GRAPHICS_API_EXT_VULKAN; break;
		case GRAPHICS_API_DIRECT3DX12: graphicsAPIExt = GRAPHICS_API_EXT_D3DX12; break;
		default:
			GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
			break;
		}
		if (FileSystemHelper::IsFolder(Application::Get().GetConfig()->GetShaderCacheDirectory()) == false)
		{
			res = FileSystemHelper::CreateFolder(Application::Get().GetConfig()->GetShaderCacheDirectory());
			GE_CORE_ASSERT(res, "Failed to create shader cache directory!");
		}
		// folder name is the file name without ext
		if(FileSystemHelper::IsFolder(cacheFolder) == false)
		{
			res = FileSystemHelper::CreateFolder(cacheFolder);
			GE_CORE_ASSERT(res, "Failed to create shader cache sub directory!");
		}

		// clear old cache files
		std::vector<std::string> oldCaches = FileSystemHelper::GetDocumentsInFolder(cacheFolder, "", "");
		for (auto& oldCache : oldCaches)
		{
			res = FileSystemHelper::DeleteDocument(oldCache);
			GE_CORE_ASSERT(res, "Failed to delete old shader cache file!");
		}

		// create a info file to indentify the shader, it can be used to check if the cache is valid when loading cache, include shader name, hash code
		if (FileSystemHelper::CreateDocument(cacheFolder + "/" + SHADER_CACHE_INFO_FILE_NAME) == false)
		{
			GE_CORE_ASSERT(false, "Failed to create shader cache info file!");
		}
		else
		{
			Serializer::Serialize(cacheFolder + "/" + SHADER_CACHE_INFO_FILE_NAME, cache);
		}
		// each pass and each stage write in sperate file
		for (int i = 0; i < shaders.size(); i++)
		{
			std::string cachePathSrc = cacheFolder + "/" + std::to_string(i);
			for (auto& [stage, code] : shaders.at(i))
			{
				// create document
				std::string cachePath = cachePathSrc + "." + stage + graphicsAPIExt;
				res = FileSystemHelper::CreateDocument(cachePath);
				res = FileSystemHelper::IsDocument(cachePath);
				GE_CORE_ASSERT(res, "Failed to create shader cache file!");
				// write machine code
				std::vector<char> data(code.size());
				memcpy(data.data(), code.data(), code.size());
				res = FileSystemHelper::WriteFile(cachePath, data);
				GE_CORE_ASSERT(res, "Failed to write shader cache file!");
			}
		}
		// each pass and each stage reflection meta write in sperate file
		for (int i = 0; i < reflectionMetas.size(); i++)
		{
			std::string cachePathSrc = cacheFolder + "/" + std::to_string(i);
			for (auto& [stage, meta] : reflectionMetas.at(i))
			{
				// create document
				std::string cachePath = cachePathSrc + "." + stage + graphicsAPIExt + SHADER_CACHE_REFLECTION_META_FILE_EXTENSION;
				res = FileSystemHelper::CreateDocument(cachePath);
				res = FileSystemHelper::IsDocument(cachePath);
				GE_CORE_ASSERT(res, "Failed to create shader reflection meta cache file!");
				// write reflection meta
				std::vector<char> data(meta.size());
				memcpy(data.data(), meta.data(), meta.size());
				res = FileSystemHelper::WriteFile(cachePath, data);
				GE_CORE_ASSERT(res, "Failed to write shader reflection meta cache file!");
			}
		}
	}

	bool Shader::LoadFromCache(std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders, 
								std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionMetas, 
								const std::string& source)
	{
		bool res = false;
		std::string graphicsAPIExt;
		std::string cacheFolder = Application::Get().GetConfig()->GetShaderCacheDirectory() + FileSystemHelper::GetDocumentNameWithoutExtension(m_FilePath);
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL: graphicsAPIExt = GRAPHICS_API_EXT_OPENGL; break;
		case GRAPHICS_API_VULKAN: graphicsAPIExt = GRAPHICS_API_EXT_VULKAN; break;
		case GRAPHICS_API_DIRECT3DX12: graphicsAPIExt = GRAPHICS_API_EXT_D3DX12; break;
		default:
			GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
			break;
		}
		if (FileSystemHelper::IsFolder(cacheFolder) == false)
		{
			return false;
		}
		//identify cache by info file, check if the shader name and hash code match, if not match, return false to recompile shader
		ShaderCacheInfo cacheInfo;
		Serializer::Deserialize(cacheFolder + "/" + SHADER_CACHE_INFO_FILE_NAME, cacheInfo);
		if (IdentifyShaderCache(source, cacheInfo) == false)
		{
			GE_CORE_INFO("Shader cache is invalid, shader source may be changed, recompile shader! Cache name: {}", cacheInfo.Name);
			return false;
		}
		// process cache, each pass and each stage read from sperate file, the file name should be in format of "pass.stage.ext", for example "0.vertex.spv"
		std::vector<std::string> cacheShaderFiles = FileSystemHelper::GetDocumentsInFolder(cacheFolder, graphicsAPIExt, "");
		std::vector<int> passIndices;
		if(cacheShaderFiles.size() == 0)
		{
			return false;
		}
		// it should from 0 to n-1, resize vector to passIndices.back() + 1 to make sure the index is valid
		for (auto& file : cacheShaderFiles)
		{
			std::string name = FileSystemHelper::GetDocumentNameWithoutExtension(file);
			size_t pos = name.find('.');
			int passIndex = std::stoi(name.substr(0, pos));
			passIndices.push_back(passIndex);
		}
		sort(passIndices.begin(), passIndices.end());
		shaders.resize(passIndices.back() + 1);
		// each pass and each stage read from sperate file
		for (auto& file : cacheShaderFiles)
		{
			std::string fileName = FileSystemHelper::GetDocumentNameWithoutExtension(file);
			size_t pos = fileName.find('.');
			std::string stage = fileName.substr(pos + 1, fileName.size() - pos - 1);
			int passIndex = std::stoi(fileName.substr(0, pos));
			// read machine code
			std::vector<char> data;
			data = FileSystemHelper::ReadFile(file);
			std::vector<std::byte> code(data.size());
			memcpy(code.data(), data.data(), data.size());
			shaders.at(passIndex)[stage] = code;
			GE_CORE_INFO("Load shader cache file: {}, pass {}, stage {}!", file, passIndex, stage);
		}
		// each pass and each stage reflection meta read from sperate file
		std::vector<std::string> cacheReflectionMetaFiles = FileSystemHelper::GetDocumentsInFolder(cacheFolder, SHADER_CACHE_REFLECTION_META_FILE_EXTENSION, graphicsAPIExt);
		reflectionMetas.resize(passIndices.back() + 1);
		for (auto& file : cacheReflectionMetaFiles)
		{
			std::string fileName = FileSystemHelper::GetDocumentNameWithoutExtension(file);
			size_t pos = fileName.find('.');
			std::string stage = fileName.substr(pos + 1, fileName.find('.', pos + 1) - pos - 1);
			int passIndex = std::stoi(fileName.substr(0, pos));
			// read reflection meta
			std::vector<char> data;
			data = FileSystemHelper::ReadFile(file);
			std::vector<std::byte> meta(data.size());
			memcpy(meta.data(), data.data(), data.size());
			reflectionMetas.at(passIndex)[stage] = meta;
			GE_CORE_INFO("Load shader reflection meta cache file: {}, pass {}, stage {}!", file, passIndex, stage);
		}
		return true;
	}

	void Shader::ReflectShader(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionMetas)
	{
		//reflection meta
		for (int i = 0; i < reflectionMetas.size(); i++)
		{
			for (auto& [stage, data] : reflectionMetas.at(i))
			{
				ShaderCompiler::Get()->Reflect(data, stage, m_PassReflections.at(i));
			}
		}
		// record properties type
		for (int i = 0; i < m_PassReflections.size(); i++)
		{
			for (auto& cbuffer : m_PassReflections.at(i).CBuffers)
			{
				for (auto& property : cbuffer.Properties)
				{
					m_PropertyTypes[property.Name] = property.Type;
				}
			}
			for (auto& resource : m_PassReflections.at(i).Resources)
			{
				m_PropertyTypes[resource.Name] = resource.Type;
			}
		}

		// calculate vertex input layout, only for vertex shader stage
		for (int i = 0; i < m_PassReflections.size(); i++)
		{
			uint32_t vertexInputStride = 0;
			uint32_t instanceInputStride = 0;
			for (int j = 0; j < m_PassReflections.at(i).VertexInputs.size(); j++)
			{
				auto& input = m_PassReflections.at(i).VertexInputs.at(j);
				// if the semantic name contains "TEXCOORD" and the semantic index is greater than or equal to 8, 
				// it is considered as instance input, otherwise it is considered as vertex input
				// such as TEXCOORD0, TEXCOORD1, TEXCOORD2, TEXCOORD3, TEXCOORD4, TEXCOORD5, TEXCOORD6, TEXCOORD7 are vertex inputs
				// TEXCOORD8, TEXCOORD9, TEXCOORD10, TEXCOORD11, TEXCOORD12, TEXCOORD13, TEXCOORD14, TEXCOORD15 are instance inputs
				if (input.SemanticName.find("TEXCOORD") != std::string::npos && input.SemanticIndex >= 8)
				{
					instanceInputStride += Utils::ShaderInputDataSize(input.Type);
					input.IsPerInstance = true;
				}
				else
				{
					vertexInputStride += Utils::ShaderInputDataSize(input.Type);
					input.IsPerInstance = false;
				}
			}
			m_PassReflections.at(i).VertexInputVertexStride = vertexInputStride;
			m_PassReflections.at(i).VertexInputInstanceStride = instanceInputStride;
		}
		// ConstPropertiesDesc

		//  ReferenceProperties
	}

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_NONE: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GRAPHICS_API_OPENGL: {
			Ref<Shader> shader = CreateRef<OpenGLShader>(path);
			if (GetShader(shader->GetShaderName()) != nullptr)
			{
				return GetShader(shader->GetShaderName());
			}
			s_Shaders[shader->GetShaderName()] = shader;
			return shader;
		}
		case GRAPHICS_API_VULKAN: {
			Ref<Shader> shader = CreateRef<VulkanShader>(path);
			if (GetShader(shader->GetShaderName()) != nullptr)
			{
				return GetShader(shader->GetShaderName());
			}
			s_Shaders[shader->GetShaderName()] = shader;
			return shader;
		}
		case GRAPHICS_API_DIRECT3DX12: {
			Ref<Shader> shader = CreateRef<D3D12Shader>(path);
			if (GetShader(shader->GetShaderName()) != nullptr)
			{
				return GetShader(shader->GetShaderName());
			}
			s_Shaders[shader->GetShaderName()] = shader;
			return shader;
		}
		}

		GE_CORE_ASSERT(false, "Unknown GraphicsAPI!");
		return nullptr;
	}
	Ref<Shader> Shader::GetShader(const std::string& name)
	{
		if (s_Shaders.find(name) != s_Shaders.end())
		{
			return s_Shaders.at(name);
		}
		return nullptr;
	}

}
#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Tools/StringHelper.h"
#include "GEngine/Tools/FileSystemHelper.h"
#include "GEngine/Tools/ShaderCompiler.h"
#include "GEngine/Application.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

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
			case SHADER_PROPERTY_TYPE_INT:		return 4;
			case SHADER_PROPERTY_TYPE_FLOAT:	return 4;
			case SHADER_PROPERTY_TYPE_VECTOR:	return 4 * 4;
			case SHADER_PROPERTY_TYPE_COLOR:	return 4 * 4;

			// ptr size
			case SHADER_PROPERTY_TYPE_SAMPLER_2D:
			case SHADER_PROPERTY_TYPE_SAMPLER_CUBE:
			case SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY:
			case SHADER_PROPERTY_TYPE_SAMPLER:
			case SHADER_PROPERTY_TYPE_TEXTURE_2D:
			case SHADER_PROPERTY_TYPE_TEXTURE_CUBE:
			case SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY:
			case SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D:
			case SHADER_PROPERTY_TYPE_STORAGE_BUFFER:
				return 4;
			}
			return 0;
		}

		CompareOperation ShaderCompareOperationFromString(const std::string& value)
		{
			if (StringHelper::ToLower(value) == VAR_NAME(less))				return COMPARE_OP_LESS;
			if (StringHelper::ToLower(value) == VAR_NAME(greater))			return COMPARE_OP_GREATER;
			if (StringHelper::ToLower(value) == VAR_NAME(lessequal))			return COMPARE_OP_LESS_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(lequal))				return COMPARE_OP_LESS_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(greaterequal))		return COMPARE_OP_GREATER_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(gequal))				return COMPARE_OP_GREATER_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(equal))				return COMPARE_OP_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(notequal))			return COMPARE_OP_NOT_EQUAL;
			if (StringHelper::ToLower(value) == VAR_NAME(always))				return COMPARE_OP_ALWAYS;

			return COMPARE_OP_LESS_EQUAL;
		}
		CullMode ShaderCullModeFromString(const std::string& value)
		{
			if (StringHelper::ToLower(value) == VAR_NAME(none))		return CULL_MODE_NONE;
			if (StringHelper::ToLower(value) == VAR_NAME(back))		return CULL_MODE_BACK;
			if (StringHelper::ToLower(value) == VAR_NAME(front))		return CULL_MODE_FRONT;
			return CULL_MODE_BACK;
		}
		
		ShaderPropertyType ShaderPropertyTypeFromString(const std::string& type)
		{
			if (StringHelper::ToLower(type) == VAR_NAME(int))					return SHADER_PROPERTY_TYPE_INT;
			if (StringHelper::ToLower(type) == VAR_NAME(float))				return SHADER_PROPERTY_TYPE_FLOAT;
			if (StringHelper::ToLower(type) == VAR_NAME(vector))				return SHADER_PROPERTY_TYPE_VECTOR;
			if (StringHelper::ToLower(type) == VAR_NAME(color))				return SHADER_PROPERTY_TYPE_COLOR;
			if (StringHelper::ToLower(type) == VAR_NAME(sampler2d))			return SHADER_PROPERTY_TYPE_SAMPLER_2D;
			if (StringHelper::ToLower(type) == VAR_NAME(samplercube))			return SHADER_PROPERTY_TYPE_SAMPLER_CUBE;
			if (StringHelper::ToLower(type) == VAR_NAME(sampler2darray))		return SHADER_PROPERTY_TYPE_SAMPLER_2D_ARRAY;
			if (StringHelper::ToLower(type) == VAR_NAME(sampler))				return SHADER_PROPERTY_TYPE_SAMPLER;
			if (StringHelper::ToLower(type) == VAR_NAME(texture2d))			return SHADER_PROPERTY_TYPE_TEXTURE_2D;
			if (StringHelper::ToLower(type) == VAR_NAME(texturecube))			return SHADER_PROPERTY_TYPE_TEXTURE_CUBE;
			if (StringHelper::ToLower(type) == VAR_NAME(texture2darray))		return SHADER_PROPERTY_TYPE_TEXTURE_2D_ARRAY;
			if (StringHelper::ToLower(type) == VAR_NAME(storageimage2d))		return SHADER_PROPERTY_TYPE_STORAGE_IMAGE_2D;
			if (StringHelper::ToLower(type) == VAR_NAME(storagebuffer))		return SHADER_PROPERTY_TYPE_STORAGE_BUFFER;
			
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return SHADER_PROPERTY_TYPE_NONE;
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

	Shader::Shader(const std::string& path, std::function<void(const std::vector<std::unordered_map<std::string, std::vector<uint32_t>>>&)> processMachingCodeFunc)
	{
		m_FilePath = path;
		FileSystemHelper::CreateFolder(Application::Get().GetConfig()->m_ShaderCacheDirectory);
		std::vector<std::string> srcCodes;
		std::vector<std::unordered_map<std::string, std::vector<uint32_t>>> shaders;
		std::string source = FileSystemHelper::ReadFileAsString(path);
		Preprocess(source, srcCodes);
		Compile(srcCodes, shaders);
		processMachingCodeFunc(shaders);
	}

	void Shader::Preprocess(const std::string& source, std::vector<std::string>& shaderSrcCode)
	{
		std::stack<int> stack;
		size_t pos = source.find("Shader", 0);
		pos = source.find("\"", pos);

		m_Name = source.substr(pos + 1, source.find("\"", pos + 1) - pos - 1);
		GE_INFO("Shader name: {}", m_Name);

		pos = source.find("{", pos);
		stack.push(1);

		pos = source.find("Properties", pos);
		pos = source.find("{", pos);
		stack.push(1);

		std::string block;
		block = source.substr(pos + 1, source.find("}", pos + 1) - pos - 1);
		//todo
		GE_INFO("Properties block {}", block);
		//properties
		//type name (float _color)...
		//split by \n or ;
		std::vector<std::string> properties = StringHelper::Split(block, ';');
		for (auto& property : properties)
		{
			auto words = StringHelper::Split(property, { ' ', '\t', '\n', '\r'});
			words = StringHelper::ClearEmptyStrings(words);
			if (words.size() == 2)
			{
				ShaderPropertyType type = Utils::ShaderPropertyTypeFromString(words[0]);
				std::string name = words[1];
				m_Properties[name] = type;
				GE_INFO("Shader property: {} {}", words[0], words[1]);
			}
		}


		pos = source.find("}", pos);
		stack.pop();
		while (source.find("Pass", pos) != std::string::npos)
		{
			m_Passes.push_back(ShaderPass());
			pos = source.find("Pass", pos);
			pos = source.find("{", pos);
			stack.push(1);

			block = source.substr(pos + 1, source.find("Program", pos + 1) - pos - 1);

			//todo
			GE_INFO("Command block {}", block);
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
				m_Passes.at(m_Passes.size() - 1).State.Cull = Utils::ShaderCullModeFromString(words[1]);
				GE_INFO("Cull mode: {}", words[1]);
			}
			//depth test
			commandPos = block.find("DepthTest", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid DepthTest command! " + commandLine);
				m_Passes.at(m_Passes.size() - 1).State.DepthTestOp = Utils::ShaderCompareOperationFromString(words[1]);
				GE_INFO("Depth test: {}", words[1]);
			}
			//depth write
			commandPos = block.find("DepthWrite", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid DepthWrite command! " + commandLine);
				m_Passes.at(m_Passes.size() - 1).State.DepthWrite = (StringHelper::ToLower(words[1]) == "true" || words[1] == "1");
				GE_INFO("Depth write: {}", words[1]);
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
					m_Passes.at(m_Passes.size() - 1).State.BlendColorSrc = Utils::ShaderBlendFactorFromString(words[1]);
					m_Passes.at(m_Passes.size() - 1).State.BlendAlphaSrc = Utils::ShaderBlendFactorFromString(words[1]);
					m_Passes.at(m_Passes.size() - 1).State.BlendColorDst = Utils::ShaderBlendFactorFromString(words[2]);
					m_Passes.at(m_Passes.size() - 1).State.BlendAlphaDst = Utils::ShaderBlendFactorFromString(words[2]);
					GE_INFO("Blend mode: {} {}", words[1], words[2]);
				}
				if (words.size() == 5)
				{
					m_Passes.at(m_Passes.size() - 1).State.BlendColorSrc = Utils::ShaderBlendFactorFromString(words[1]);
					m_Passes.at(m_Passes.size() - 1).State.BlendAlphaSrc = Utils::ShaderBlendFactorFromString(words[3]);
					m_Passes.at(m_Passes.size() - 1).State.BlendColorDst = Utils::ShaderBlendFactorFromString(words[2]);
					m_Passes.at(m_Passes.size() - 1).State.BlendAlphaDst = Utils::ShaderBlendFactorFromString(words[4]);
					GE_INFO("Blend mode: {} {} {} {}", words[1], words[2], words[3], words[4]);
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
					m_Passes.at(m_Passes.size() - 1).State.BlendColor = Utils::ShaderBlendModeFromString(words[1]);
					m_Passes.at(m_Passes.size() - 1).State.BlendAlpha = Utils::ShaderBlendModeFromString(words[1]);
					GE_INFO("Blend op: {}", words[1]);
				}
				if (words.size() == 3)
				{
					m_Passes.at(m_Passes.size() - 1).State.BlendColor = Utils::ShaderBlendModeFromString(words[1]);
					m_Passes.at(m_Passes.size() - 1).State.BlendAlpha = Utils::ShaderBlendModeFromString(words[2]);
					GE_INFO("Blend op: {} {}", words[1], words[2]);
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
				m_Passes.at(m_Passes.size() - 1).State.ColorMask = mask;
				GE_INFO("Color mask: {}", mask);
			}
			//tag
			commandPos = block.find("Tag", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', '\r'});
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid Tag command! " + commandLine);
				m_Passes.at(m_Passes.size() - 1).State.Tag = words[1];
				GE_INFO("Tag: {}", words[1]);
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

			// todo
			GE_INFO("Program block {}", block);
			// src code
			shaderSrcCode.push_back(block);

			pos = source.find("}", pos + 1);
			stack.pop();
		}
		
	}

	bool Shader::Compile(const std::vector<std::string>& shaderSrcCodes, std::vector<std::unordered_map<std::string, std::vector<uint32_t>>>& shaders)
	{
		for (int i = 0; i < m_StageEntryPoints.size(); i++)
		{
			GE_CORE_ASSERT(i < shaderSrcCodes.size(), "Shader source code size mismatch!");
			GE_CORE_ASSERT(m_StageEntryPoints.at(i).size() > 0, "No entry points found for shader pass " + std::to_string(i));

			std::vector<uint32_t> machineCode;
			for (auto&& [stage, entryPoint] : m_StageEntryPoints.at(i))
			{
				bool result = ShaderCompiler::Get()->Compile(shaderSrcCodes.at(i), stage, entryPoint, machineCode);
				GE_CORE_ASSERT(result, "Failed to compile shader stage " + stage + " for pass " + std::to_string(i));
			}
		}
		GE_CORE_ASSERT(false, "");
		//reflection

		// ConstPropertiesDesc

		//  ReferenceProperties
		return true;
	}

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::GRAPHICS_API_None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::GRAPHICS_API_OpenGL: {
			Ref<Shader> shader = CreateRef<OpenGLShader>(path);
			if (GetShader(shader->GetShaderName()) != nullptr)
			{
				return GetShader(shader->GetShaderName());
			}
			s_Shaders[shader->GetShaderName()] = shader;
			return shader;
		}
		case GraphicsAPI::GRAPHICS_API_Vulkan: {
			Ref<Shader> shader = CreateRef<VulkanShader>(path);
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
#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Tools/StringHelper.h"
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
			case ShaderPropertyType::Int:		return 4;
			case ShaderPropertyType::Float:		return 4;
			case ShaderPropertyType::Vector:	return 4 * 4;
			case ShaderPropertyType::Color:		return 4 * 4;

			// ptr size
			case ShaderPropertyType::Sampler2D:
			case ShaderPropertyType::SamplerCube:
			case ShaderPropertyType::Sampler2DArray:
			case ShaderPropertyType::Sampler:
			case ShaderPropertyType::Texture2D:
			case ShaderPropertyType::TextureCube:
			case ShaderPropertyType::Texture2DArray:
			case ShaderPropertyType::StorageImage2D:
			case ShaderPropertyType::StorageBuffer:
				return 4;
			}
			return 0;
		}

		CompareOperation ShaderCompareOperationFromString(const std::string& value)
		{
			if (StringHelper::ToLower(value) == VAR_NAME(less))				return CompareOperation::Less;
			if (StringHelper::ToLower(value) == VAR_NAME(greater))			return CompareOperation::Greater;
			if (StringHelper::ToLower(value) == VAR_NAME(lessequal))			return CompareOperation::LessEqual;
			if (StringHelper::ToLower(value) == VAR_NAME(lequal))				return CompareOperation::LessEqual;
			if (StringHelper::ToLower(value) == VAR_NAME(greaterequal))		return CompareOperation::GreaterEqual;
			if (StringHelper::ToLower(value) == VAR_NAME(gequal))				return CompareOperation::GreaterEqual;
			if (StringHelper::ToLower(value) == VAR_NAME(equal))				return CompareOperation::Equal;
			if (StringHelper::ToLower(value) == VAR_NAME(notequal))			return CompareOperation::NotEqual;
			if (StringHelper::ToLower(value) == VAR_NAME(always))				return CompareOperation::Always;

			return CompareOperation::LessEqual;
		}
		CullMode ShaderCullModeFromString(const std::string& value)
		{
			if (StringHelper::ToLower(value) == VAR_NAME(none))		return CullMode::None;
			if (StringHelper::ToLower(value) == VAR_NAME(back))		return CullMode::Back;
			if (StringHelper::ToLower(value) == VAR_NAME(front))		return CullMode::Front;
			return CullMode::Back;
		}
		
		ShaderPropertyType ShaderPropertyTypeFromString(const std::string& type)
		{
			if (StringHelper::ToLower(type) == VAR_NAME(int))					return ShaderPropertyType::Int;
			if (StringHelper::ToLower(type) == VAR_NAME(float))				return ShaderPropertyType::Float;
			if (StringHelper::ToLower(type) == VAR_NAME(vector))				return ShaderPropertyType::Vector;
			if (StringHelper::ToLower(type) == VAR_NAME(color))				return ShaderPropertyType::Color;
			if (StringHelper::ToLower(type) == VAR_NAME(sampler2d))			return ShaderPropertyType::Sampler2D;
			if (StringHelper::ToLower(type) == VAR_NAME(samplercube))			return ShaderPropertyType::SamplerCube;
			if (StringHelper::ToLower(type) == VAR_NAME(storageimage2d))		return ShaderPropertyType::StorageImage2D;
			if (StringHelper::ToLower(type) == VAR_NAME(storagebuffer))		return ShaderPropertyType::StorageBuffer;
			if (StringHelper::ToLower(type) == VAR_NAME(texture2d))			return ShaderPropertyType::Texture2D;
			if (StringHelper::ToLower(type) == VAR_NAME(sampler))				return ShaderPropertyType::Sampler;
			if (StringHelper::ToLower(type) == VAR_NAME(sampler2darray))		return ShaderPropertyType::Sampler2DArray;
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return ShaderPropertyType::None;
		}
		BlendFactor ShaderBlendFactorFromString(const std::string& factor)
		{
			if (StringHelper::ToUpper(factor) == VAR_NAME(SRCALPHA))			return BlendFactor::SRC_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(DSTALPHA))			return BlendFactor::DST_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(SRCCOLOR))			return BlendFactor::SRC_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(DSTCOLOR))			return BlendFactor::DST_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSSRCALPHA))	return BlendFactor::ONE_MINUS_SRC_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSDSTALPHA))	return BlendFactor::ONE_MINUS_DST_ALPHA;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSSRCCOLOR))	return BlendFactor::ONE_MINUS_SRC_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONEMINUSDSTCOLOR))	return BlendFactor::ONE_MINUS_DST_COLOR;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ONE))				return BlendFactor::ONE;
			if (StringHelper::ToUpper(factor) == VAR_NAME(ZERO))				return BlendFactor::ZERO;

			GE_CORE_ASSERT(false, "Unknown blend factor! " + factor);
		}
		BlendMode ShaderBlendModeFromString(const std::string& type)
		{
			if (StringHelper::ToLower(type) == VAR_NAME(none))				return BlendMode::None;
			if (StringHelper::ToLower(type) == VAR_NAME(add))					return BlendMode::Add;
			if (StringHelper::ToLower(type) == VAR_NAME(substract))			return BlendMode::Substract;
			if (StringHelper::ToLower(type) == VAR_NAME(reversesubstract))	return BlendMode::ReverseSubstract;
			if (StringHelper::ToLower(type) == VAR_NAME(min))					return BlendMode::Min;
			if (StringHelper::ToLower(type) == VAR_NAME(max))					return BlendMode::Max;

			GE_CORE_ASSERT(false, "Unknown blend type! " + type);
			return BlendMode::None;
		}
		const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Assets/Cache/Shaders";
		}
		void SetShaderMacroBool(std::string& source, const std::string& macro, bool value)
		{
			//第二行插入
			size_t eol = source.find_first_of("\n", 0);
			source.insert(eol + 1, "#define " + macro + " " + (value ? "1" : "0") + "\n");
		}
		void SetShaderMacroExpression(std::string& source, const std::string& macro, std::string& exp)
		{
			//第二行插入
			size_t eol = source.find_first_of("\n", 0);
			source.insert(eol + 1, "#define " + macro + " " + exp + "\n");
		}
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
		std::vector<std::string> properties = StringHelper::Split(block, {'\n', ';'});
		for (auto& property : properties)
		{
			auto words = StringHelper::Split(property, { ' ', '\t' });
			words = StringHelper::ClearEmptyStrings(words);
			GE_CORE_ASSERT(words.size() == 2, "Invalid property declaration! " + property);
			ShaderPropertyType type = Utils::ShaderPropertyTypeFromString(words[0]);
			std::string name = words[1];
			m_Properties[name] = type;
			GE_INFO("Shader property: {} {}", words[0], words[1]);
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
				auto words = StringHelper::Split(commandLine, { ' ', '\t' });
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
				auto words = StringHelper::Split(commandLine, { ' ', '\t' });
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
				auto words = StringHelper::Split(commandLine, { ' ', '\t' });
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
				auto words = StringHelper::Split(commandLine, { ' ', '\t' });
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
				auto words = StringHelper::Split(commandLine, { ' ', '\t' });
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
				auto words = StringHelper::Split(commandLine, { ' ', '\t', ',' });
				words = StringHelper::ClearEmptyStrings(words);
				int mask = 0;
				for (size_t i = 1; i < words.size(); i++)
				{
					if (StringHelper::ToUpper(words[i]) == "R") mask |= ColorMaskChannel::R;
					if (StringHelper::ToUpper(words[i]) == "G") mask |= ColorMaskChannel::G;
					if (StringHelper::ToUpper(words[i]) == "B") mask |= ColorMaskChannel::B;
					if (StringHelper::ToUpper(words[i]) == "A") mask |= ColorMaskChannel::A;
				}
				m_Passes.at(m_Passes.size() - 1).State.ColorMask = mask;
				GE_INFO("Color mask: {}", mask);
			}
			//tag
			commandPos = block.find("Tag", 0);
			if (commandPos != std::string::npos)
			{
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t' });
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 2, "Invalid Tag command! " + commandLine);
				m_Passes.at(m_Passes.size() - 1).State.Tag = words[1];
				GE_INFO("Tag: {}", words[1]);
			}
			//#pragma
			commandPos = 0;
			m_StageFuncNames.push_back(std::unordered_map<std::string, std::string>());
			while (block.find("#pragma", commandPos) != std::string::npos)
			{
				commandPos = block.find("#pragma", commandPos);
				commandLine = block.substr(commandPos, block.find("\n", commandPos) - commandPos);
				auto words = StringHelper::Split(commandLine, { ' ', '\t', ',' });
				words = StringHelper::ClearEmptyStrings(words);
				GE_CORE_ASSERT(words.size() == 3, "Invalid pragma command! " + commandLine);
				if (StringHelper::ToUpper(words[1]) == "VERTEX")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_VERTEX] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "FRAGMENT")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_FRAGMENT] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "HULL")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_HULL] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "DOMAIN")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_DOMAIN] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "GEOMETRY")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_GEOMETRY] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "COMPUTE")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_COMPUTE] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "AMPLIFICATION")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_AMPLIFICATION] = words[2];
				else if (StringHelper::ToUpper(words[1]) == "MESH")
					m_StageFuncNames.at(m_StageFuncNames.size() - 1)[SHADER_STAGE_MESH] = words[2];
				else
					GE_CORE_ASSERT(false, "Unknown shader stage in pragma command! " + commandLine);
				commandPos++;
			}
			


			pos = source.find("Program", pos);
			pos = source.find("{", pos);
			stack.push(1);
			size_t begin = pos + 1;
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
			}
			block = source.substr(begin, pos - 1);

			// todo
			GE_INFO("Program block {}", block);
			// src code

			//reflection

			// ConstPropertiesDesc

			//  ReferenceProperties
			pos = source.find("}", pos + 1);
			stack.pop();
		}
		
	}

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Graphics::GetGraphicsAPI())
		{
		case GraphicsAPI::API::None: {
			GE_CORE_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		}
		case GraphicsAPI::API::OpenGL: {
			Ref<Shader> shader = CreateRef<OpenGLShader>(path);
			if (GetShader(shader->GetShaderName()) != nullptr)
			{
				return GetShader(shader->GetShaderName());
			}
			s_Shaders[shader->GetShaderName()] = shader;
			return shader;
		}
		case GraphicsAPI::API::Vulkan: {
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
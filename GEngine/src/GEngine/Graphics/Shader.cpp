#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Graphics/Renderer.h"
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
	std::string		const	ShaderMacroName::GE_GRAPHICS_API				= VAR_NAME(GE_GRAPHICS_API);
	

	std::string		const	ShaderDataFlag::None							= VAR_NAME(#None);
	std::string		const	ShaderDataFlag::Name							= VAR_NAME(#Name);
	std::string		const	ShaderDataFlag::Blend							= VAR_NAME(#Blend);
	std::string		const	ShaderDataFlag::DepthMask						= VAR_NAME(#DepthMask);
	std::string		const	ShaderDataFlag::DepthTest						= VAR_NAME(#DepthTest);
	std::string		const	ShaderDataFlag::Properties						= VAR_NAME(#Properties);
	std::string		const	ShaderDataFlag::Type							= VAR_NAME(#Type);
					
	std::string     const	ShaderStage::Vertex								= VAR_NAME(vertex);
	std::string     const	ShaderStage::Fragment							= VAR_NAME(fragment);
	std::string     const	ShaderStage::Pixel								= VAR_NAME(pixel);
	std::string		const	ShaderStage::Compute							= VAR_NAME(compute);
	std::string		const	ShaderStage::TessellationControl				= VAR_NAME(tessellationcontrol);
	std::string		const	ShaderStage::TessellationEvaluation				= VAR_NAME(tessellationevaluation);
	std::string		const	ShaderStage::Geometry							= VAR_NAME(geometry);

	namespace Utils
	{
		uint32_t ShaderUniformTypeSize(ShaderUniformType type)
		{
			switch (type)
			{
			case ShaderUniformType::Int:	return 4;
			case ShaderUniformType::Float:	return 4;
			case ShaderUniformType::Vector: return 4 * 4;
			case ShaderUniformType::Color:	return 4 * 4;
			case ShaderUniformType::Mat3:	return 4 * 4 * 3;
			case ShaderUniformType::Mat4:	return 4 * 4 * 4;
			}
			return 0;
		}
		std::string ShaderUniformTypeToGlslType(ShaderUniformType type)
		{
			switch (type)
			{
			case ShaderUniformType::None:
				break;
			case ShaderUniformType::Int:
				return VAR_NAME(int);
			case ShaderUniformType::Float:
				return VAR_NAME(float);
			case ShaderUniformType::Vector:
				return VAR_NAME(vec4);
			case ShaderUniformType::Color:
				return VAR_NAME(vec4);
			case ShaderUniformType::Mat3:
				return VAR_NAME(mat3);
			case ShaderUniformType::Mat4:
				return VAR_NAME(mat4);
			case ShaderUniformType::Sampler2D:
				break;
			case ShaderUniformType::SamplerCube:
				break;
			case ShaderUniformType::StorageImage2D:
				break;
			case ShaderUniformType::StorageBuffer:
				break;
			default:
				break;
			}
		}
		
		std::string ToLower(std::string string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::tolower);
			return string;
		}
		std::string ToUpper(std::string string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::toupper);
			return string;
		}
		uint8_t ShaderVertexInputFlagToLocation(const std::string& flag)
		{
			if (ToLower(flag) == VAR_NAME(position))	return 0;
			if (ToLower(flag) == VAR_NAME(color))		return 1;
			if (ToLower(flag) == VAR_NAME(normal))		return 2;
			if (ToLower(flag) == VAR_NAME(tangent))		return 3;
			if (ToLower(flag) == VAR_NAME(uv0))			return 4;
			if (ToLower(flag) == VAR_NAME(uv1))			return 5;
			if (ToLower(flag) == VAR_NAME(uv2))			return 6;
			if (ToLower(flag) == VAR_NAME(uv3))			return 7;
			GE_CORE_ASSERT(false, "invalid vertex input");
		}
		bool ShaderBoolFromString(const std::string& value)
		{
			if (ToLower(value) == VAR_NAME(on))				return true;
			if (ToLower(value) == VAR_NAME(1))				return true;
			if (ToLower(value) == VAR_NAME(true))			return true;
			return false;
		}
		std::string ShaderTypeFromString(const std::string& type)
		{
			if (ToLower(type) == ShaderStage::Vertex)					return ShaderStage::Vertex;
			if (ToLower(type) == ShaderStage::Fragment || 
				ToLower(type) == ShaderStage::Pixel)					return ShaderStage::Fragment;
			if (ToLower(type) == ShaderStage::Compute)					return ShaderStage::Compute;
			if (ToLower(type) == ShaderStage::TessellationControl)		return ShaderStage::TessellationControl;
			if (ToLower(type) == ShaderStage::TessellationEvaluation)	return ShaderStage::TessellationEvaluation;
			if (ToLower(type) == ShaderStage::Geometry)					return ShaderStage::Geometry;

			GE_CORE_ASSERT(false, "Unknown shader type!");
			return "";
		}
		CompareOperation ShaderCompareOperationFromString(const std::string& value)
		{
			if (ToLower(value) == VAR_NAME(less))				return CompareOperation::Less;
			if (ToLower(value) == VAR_NAME(greater))			return CompareOperation::Greater;
			if (ToLower(value) == VAR_NAME(lessequal))			return CompareOperation::LessEqual;
			if (ToLower(value) == VAR_NAME(lequal))				return CompareOperation::LessEqual;
			if (ToLower(value) == VAR_NAME(greaterequal))		return CompareOperation::GreaterEqual;
			if (ToLower(value) == VAR_NAME(gequal))				return CompareOperation::GreaterEqual;
			if (ToLower(value) == VAR_NAME(equal))				return CompareOperation::Equal;
			if (ToLower(value) == VAR_NAME(notequal))			return CompareOperation::NotEqual;
			if (ToLower(value) == VAR_NAME(always))				return CompareOperation::Always;

			return CompareOperation::LessEqual;
		}
		CullMode ShaderCullModeFromString(const std::string& value)
		{
			if (ToLower(value) == VAR_NAME(none)) return CullMode::None;
			if (ToLower(value) == VAR_NAME(back)) return CullMode::Back;
			if (ToLower(value) == VAR_NAME(front)) return CullMode::Front;
			return CullMode::Back;
		}
		std::vector<std::string> SplitString(const std::string& string, char delimiter)
		{
			std::vector<std::string> result;
			std::stringstream ss(string);
			std::string item;
			while (std::getline(ss, item, delimiter))
			{
				item = item.substr(0, item.size());
				if (item.empty() == false)
					result.push_back(item);
			}
			return result;
		}
		std::string RemoveCharFromString(const std::string& string, char character)
		{
			std::string result = string;
			for (auto it = result.begin(); it != result.end();)
			{
				if (*it == character)
					it = result.erase(it);
				else
					++it;
			}
			return result;
		}
		std::string RemoveCharFromStringInHead(const std::string& string, char character)
		{
			std::string result = string;
			for (auto it = result.begin(); it != result.end();)
			{
				if (*it == character)
					it = result.erase(it);
				else
					return result;
			}
			return result;
		}
		std::string RemoveCharFromStringInTail(const std::string& string, char character)
		{
			std::string result = string;
			for (int i = result.size() - 1; i >= 0; i--)
			{
				if (result.at(i) == character)
					result.erase(i);
				else
					return result;
			}
			return result;
		}
		ShaderUniformType ShaderUniformTypeFromString(const std::string& type)
		{
			if (ToLower(type) == VAR_NAME(int))					return ShaderUniformType::Int;
			if (ToLower(type) == VAR_NAME(float))				return ShaderUniformType::Float;
			if (ToLower(type) == VAR_NAME(vector))				return ShaderUniformType::Vector;
			if (ToLower(type) == VAR_NAME(color))				return ShaderUniformType::Color;
			if (ToLower(type) == VAR_NAME(mat3))				return ShaderUniformType::Mat3;
			if (ToLower(type) == VAR_NAME(mat4))				return ShaderUniformType::Mat4;
			if (ToLower(type) == VAR_NAME(sampler2d))			return ShaderUniformType::Sampler2D;
			if (ToLower(type) == VAR_NAME(samplercube))			return ShaderUniformType::SamplerCube;
			if (ToLower(type) == VAR_NAME(storageimage2d))		return ShaderUniformType::StorageImage2D;
			if (ToLower(type) == VAR_NAME(storagebuffer))		return ShaderUniformType::StorageBuffer;
			if (ToLower(type) == VAR_NAME(texture2d))			return ShaderUniformType::Texture2D;
			if (ToLower(type) == VAR_NAME(sampler))				return ShaderUniformType::Sampler;
			if (ToLower(type) == VAR_NAME(sampler2darray))		return ShaderUniformType::Sampler2DArray;
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return ShaderUniformType::None;
		}
		BlendFactor ShaderBlendFactorFromString(const std::string& factor)
		{
			if (ToUpper(factor) == VAR_NAME(SRCALPHA))			return BlendFactor::SRC_ALPHA;
			if (ToUpper(factor) == VAR_NAME(DSTALPHA))			return BlendFactor::DST_ALPHA;
			if (ToUpper(factor) == VAR_NAME(SRCCOLOR))			return BlendFactor::SRC_COLOR;
			if (ToUpper(factor) == VAR_NAME(DSTCOLOR))			return BlendFactor::DST_COLOR;
			if (ToUpper(factor) == VAR_NAME(ONEMINUSSRCALPHA))	return BlendFactor::ONE_MINUS_SRC_ALPHA;
			if (ToUpper(factor) == VAR_NAME(ONEMINUSDSTALPHA))	return BlendFactor::ONE_MINUS_DST_ALPHA;
			if (ToUpper(factor) == VAR_NAME(ONEMINUSSRCCOLOR))	return BlendFactor::ONE_MINUS_SRC_COLOR;
			if (ToUpper(factor) == VAR_NAME(ONEMINUSDSTCOLOR))	return BlendFactor::ONE_MINUS_DST_COLOR;
			if (ToUpper(factor) == VAR_NAME(ONE))				return BlendFactor::ONE;
			if (ToUpper(factor) == VAR_NAME(ZERO))				return BlendFactor::ZERO;

			GE_CORE_ASSERT(false, "Unknown blend factor! " + factor);
		}
		BlendMode ShaderBlendModeFromString(const std::string& type)
		{
			if (ToLower(type) == VAR_NAME(none))				return BlendMode::None;
			if (ToLower(type) == VAR_NAME(add))					return BlendMode::Add;
			if (ToLower(type) == VAR_NAME(substract))			return BlendMode::Substract;
			if (ToLower(type) == VAR_NAME(reversesubstract))	return BlendMode::ReverseSubstract;
			if (ToLower(type) == VAR_NAME(min))					return BlendMode::Min;
			if (ToLower(type) == VAR_NAME(max))					return BlendMode::Max;

			GE_CORE_ASSERT(false, "Unknown blend type! " + type);
			return BlendMode::None;
		}
		const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Assets/Cache/Shaders";
		}
		void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
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
		void InsertShaderVertexInput(std::string& source, const std::string& vertexInput)
		{
			//第二行插入
			size_t eol = source.find_first_of("\n", 0);
			source.insert(eol + 1, vertexInput);
		}
		std::string ProcessShaderName(const std::string& source)
		{
			std::string output;

			const char* token = "#Name";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return output;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string name = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = name.find(' ', index)) != std::string::npos)
			{
				name.erase(index, 1);
			}
			output = name;
			GE_CORE_INFO("Shader name: {0}", name);
			return output;
		}
		void ProcessShaderBlend(const std::string& source, BlendMode& modeColor, BlendMode modeAlpha,
			BlendFactor& srcColor, BlendFactor& dstColor, BlendFactor& srcAlpha, BlendFactor& dstAlpha)
		{
			const char* token = "#Blend";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string blendString = source.substr(begin, eol - begin);
			blendString = Utils::RemoveCharFromString(blendString, ';');
			blendString = Utils::RemoveCharFromString(blendString, '\r');
			blendString = Utils::RemoveCharFromString(blendString, '\n');
			std::vector<std::string> blends = Utils::SplitString(blendString, ' ');
			GE_CORE_ASSERT(blends.size() == 3 || blends.size() == 1 || blends.size() == 6, "Syntax error");
			modeColor = Utils::ShaderBlendModeFromString(blends.at(0));
			modeAlpha = Utils::ShaderBlendModeFromString(blends.at(0));
			if (blends.size() == 3)
			{
				modeColor = Utils::ShaderBlendModeFromString(blends.at(0));
				modeAlpha = Utils::ShaderBlendModeFromString(blends.at(0));
				srcColor = Utils::ShaderBlendFactorFromString(blends.at(1));
				srcAlpha = Utils::ShaderBlendFactorFromString(blends.at(1));
				dstColor = Utils::ShaderBlendFactorFromString(blends.at(2));
				dstAlpha = Utils::ShaderBlendFactorFromString(blends.at(2));
				GE_CORE_TRACE("Blend type: {0}, Src factor: {1}, Dst factor: {2}", blends.at(0), blends.at(1), blends.at(2));
			}
			else if (blends.size() == 6)
			{
				modeColor = Utils::ShaderBlendModeFromString(blends.at(0));
				srcColor = Utils::ShaderBlendFactorFromString(blends.at(1));
				dstColor = Utils::ShaderBlendFactorFromString(blends.at(2));
				modeAlpha = Utils::ShaderBlendModeFromString(blends.at(3));
				srcAlpha = Utils::ShaderBlendFactorFromString(blends.at(4));
				dstAlpha = Utils::ShaderBlendFactorFromString(blends.at(5));
			}
			else
			{
				GE_CORE_TRACE("Blend type: {0}", blends.at(0));
			}
		}
		void ProcessShaderDepthWrite(const std::string& source, bool& enableDepthWrite)
		{
			const char* token = "#DepthWrite";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string depthMaskProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = depthMaskProp.find(' ', index)) != std::string::npos)
			{
				depthMaskProp.erase(index, 1);
			}
			enableDepthWrite = Utils::ShaderBoolFromString(depthMaskProp);
			GE_CORE_TRACE("DepthWrite: {0}", enableDepthWrite);
		}
		void ProcessShaderDepthTest(const std::string& source, CompareOperation& depthTestOperation)
		{
			const char* token = "#DepthTest";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string depthTestProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = depthTestProp.find(' ', index)) != std::string::npos)
			{
				depthTestProp.erase(index, 1);
			}
			depthTestOperation = Utils::ShaderCompareOperationFromString(depthTestProp);
			GE_CORE_TRACE("DepthTest: {0}", depthTestProp);
		}
		void ProcessShaderCull(const std::string& source, CullMode& mode)
		{
			const char* token = "#Cull";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + tokenLength + 1;
			std::string cullProp = source.substr(begin, eol - begin);
			int index = 0;
			while ((index = cullProp.find(' ', index)) != std::string::npos)
			{
				cullProp.erase(index, 1);
			}
			mode = Utils::ShaderCullModeFromString(cullProp);
			GE_CORE_TRACE("Cull: {0}", cullProp);
		}
		std::string ProcessShaderVertexInput(const std::string& source)
		{
			std::string output;

			const char* token = "#VertexInput";
			const char* endToken = "#EndVertexInput";
			size_t tokenLength = strlen(token);
			size_t pos = source.find(token, 0);
			if (pos == std::string::npos)
			{
				return output;
			}
			size_t eol = source.find_first_of("\r\n", pos);
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			size_t end = source.find(endToken, pos);
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
			for (auto prop : props)
			{
				std::string flag = Utils::SplitString(prop, ':').at(1);
				std::string pro = Utils::SplitString(prop, ':').at(0);
				output += "layout(location=" + std::to_string(Utils::ShaderVertexInputFlagToLocation(flag)) + ") in " + pro + ";\n";
			}
			GE_CORE_INFO(output);
			return output;
		}
		void ProcessShaderBlocks(const std::string& source, std::unordered_map<std::string, std::string>& blocks)
		{
			const char* token			= "#BeginBlock";
			const char* endToken		= "#EndBlock";

			size_t pos					= 0;
			pos							= source.find(token, pos);
			while (pos != source.npos)
			{
				size_t elo				= source.find_first_of("\r\n", pos);
				GE_CORE_ASSERT(elo != std::string::npos, "Syntax error");
				size_t begin			= pos + strlen(token) + 1;
				std::string blockName	= source.substr(begin, elo - begin);
				blockName				= RemoveCharFromString(blockName, ' ');
				begin					= source.find_first_not_of("\r\n", elo);
				pos						= source.find(endToken, begin);
				GE_CORE_ASSERT(pos != std::string::npos, "Syntax error");
				std::string content		= source.substr(begin, pos - begin);
				GE_CORE_ASSERT(blocks.find(blockName) == blocks.end(), "block name {} is used", blockName);
				blocks[blockName]		= content;
				pos						= source.find(token, pos);
			}
			
		}
		void ProcessShaderPasses(const std::string& source, std::unordered_map<std::string, std::string>& blocks, std::unordered_map<std::string, ShaderPass>& passes)
		{
			const char* token			= "##Pass";
			const char* endToken		= "##EndPass";
			size_t pos					= 0;
			pos							= source.find(token, pos);
			while (pos != source.npos)
			{
				size_t elo				= source.find_first_of("\r\n", pos);
				GE_CORE_ASSERT(elo != std::string::npos, "Syntax error");
				size_t begin			= pos + strlen(token) + 1;
				std::string passName	= source.substr(begin, elo - begin);
				passName				= RemoveCharFromString(passName, ' ');
				begin					= source.find_first_not_of("\r\n", elo);
				pos						= source.find(endToken, begin);
				GE_CORE_ASSERT(pos != std::string::npos, "Syntax error");
				std::string content		= source.substr(begin, pos - begin);
				ShaderPass pass{};
				ProcessShaderBlend(content, pass.State.BlendColor, pass.State.BlendAlpha, 
					pass.State.BlendColorSrc, pass.State.BlendColorDst, pass.State.BlendAlphaSrc, pass.State.BlendAlphaDst);
				ProcessShaderDepthWrite(content, pass.State.ColorWrite);
				ProcessShaderDepthTest(content, pass.State.DepthTestOp);
				ProcessShaderCull(content, pass.State.Cull);
				std::string blockName;
				if (ProcessShaderStage(content, ShaderStage::Vertex, blockName))
				{
					pass.Stages[ShaderStage::Vertex] = blocks[blockName];
				}
				if (ProcessShaderStage(content, ShaderStage::Fragment, blockName))
				{
					pass.Stages[ShaderStage::Fragment] = blocks[blockName];
				}
				if (ProcessShaderStage(content, ShaderStage::Pixel, blockName))
				{
					pass.Stages[ShaderStage::Fragment] = blocks[blockName];
				}
				if (ProcessShaderStage(content, ShaderStage::Compute, blockName))
				{
					pass.Stages[ShaderStage::Compute] = blocks[blockName];
				}
				if (ProcessShaderStage(content, ShaderStage::Geometry, blockName))
				{
					pass.Stages[ShaderStage::Geometry] = blocks[blockName];
				}
				if (ProcessShaderStage(content, ShaderStage::TessellationControl, blockName))
				{
					pass.Stages[ShaderStage::TessellationControl] = blocks[blockName];
				}
				if (ProcessShaderStage(content, ShaderStage::TessellationEvaluation, blockName))
				{
					pass.Stages[ShaderStage::TessellationEvaluation] = blocks[blockName];
				}
				GE_CORE_ASSERT(passes.find(passName) == passes.end(), "pass name {} is uesd", passName);
				passes[passName]		= pass;
				pos						= source.find(token, pos);
			}
		}
		bool ProcessShaderStage(const std::string& source, const std::string& stage, std::string& blockName)
		{
			const char* token			= stage.c_str();
			size_t tokenLength			= strlen(token);
			size_t pos					= source.find(token, 0);
			if (pos == std::string::npos)
			{
				return false;
			}
			size_t eol					= source.find_first_of("\r\n", pos);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin				= pos + tokenLength + 1;
			blockName					= source.substr(begin, eol - begin);
			RemoveCharFromString(blockName, ' ');
			return true;
		}
		std::string ReadFile(const std::string& path)
		{
			std::string src;
			std::ifstream file(path, std::ios::in | std::ios::binary);

			GE_CORE_ASSERT(file, " file " + path + " not found");

			file.seekg(0, std::ios::end);
			src.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(&src[0], src.size());
			file.close();
			return src;
		}
		void ProcessShaderProperties(const std::string& source, std::vector<ShaderUniform>& uniformCache, std::vector<ShaderUniformTexture2D>& texture2DCache,
			std::vector<ShaderUniformCubeMap>& cubeMapCache, std::vector<ShaderUniformStorageImage2D>& storageImage2DCache,
			std::vector<ShaderUniformStorageBuffer>& storageBufferCache, uint32_t locationStart, uint32_t slotStart, uint32_t storageBufferSlotOffset)
		{
			const char* propertyToken = "#Properties";
			const char* propertyEndToken = "#EndProperties";
			size_t propertyTokenLength = strlen(propertyToken);

			uint32_t location = locationStart;
			uint32_t slot = slotStart;

			// 插入模型矩阵
			ShaderUniform modelUbo;
			modelUbo.Name = "GE_MATRIX_M";
			modelUbo.Type = ShaderUniformType::Mat4;
			modelUbo.Size = ShaderUniformTypeSize(ShaderUniformType::Mat4);
			modelUbo.Location = location;
			location += modelUbo.Size;
			uniformCache.push_back(modelUbo);
			GE_CORE_TRACE("Property Name: {0}, Property Size: {1}, Property Location: {2}", modelUbo.Name, modelUbo.Size, modelUbo.Location);

			size_t pos = source.find(propertyToken, 0);
			if (pos != std::string::npos)
			{
				// split properties by \n
				size_t eol = source.find_first_of("\r\n", pos);
				GE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
				size_t nextLinePos = source.find_first_not_of("\r\n", eol);
				size_t end = source.find(propertyEndToken, pos);
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


				// split properties by :
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
						uniform.Name = propName;
						uniform.Type = Utils::ShaderUniformTypeFromString(propType);
						uniform.Size = ShaderUniformTypeSize(uniform.Type);
						uniform.Location = location;
						location += uniform.Size;
						uniformCache.push_back(uniform);
						GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property Size: {2}, Property Location: {3}", uniform.Name, propType, uniform.Size, uniform.Location);
					}
					if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::Sampler2D)
					{
						ShaderUniformTexture2D	uniform;
						uniform.Name = propName;
						uniform.Slot = slot;
						uniform.Texture = Texture2D::White();

						GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property slot: {2}", uniform.Name, propType, slot);

						slot++;
						texture2DCache.push_back(uniform);
					}
					if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::SamplerCube)
					{
						ShaderUniformCubeMap	uniform;
						uniform.Name = propName;
						uniform.Slot = slot;
						uniform.Cubemap = CubeMap::White();

						GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property slot: {2}", uniform.Name, propType, slot);

						slot++;
						cubeMapCache.push_back(uniform);
					}
					if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::StorageImage2D)
					{
						ShaderUniformStorageImage2D	uniform;
						uniform.Name = propName;
						uniform.Slot = slot;
						uniform.Image = nullptr;

						GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property slot: {2}", uniform.Name, propType, slot);

						slot++;
						storageImage2DCache.push_back(uniform);
					}
					if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::StorageBuffer)
					{
						ShaderUniformStorageBuffer	uniform;
						uniform.Name = propName;
						uniform.Slot = slot + storageBufferSlotOffset;
						uniform.Buffer = nullptr;

						GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property binding: {2}", uniform.Name, propType, uniform.Slot);

						slot++;
						storageBufferCache.push_back(uniform);
					}
				}
			}
		}
	}
	

	void Shader::Preprocess(std::string& source)
	{
		SetGlslVersionString(source);
		SetMacroBool(source);
		SetMacroExp(source);
		SetMacroMaterialDefine(source);
	}

	void Shader::SetGlslVersionString(std::string& source)
	{
		source = "#version 450 core\n" + source;
	}

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
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
	void Shader::SetMacroMaterialDefine(std::string& source)
	{
		std::string macro = ShaderMacroName::GE_MATERIAL_DEFINE;
		std::string exp = "layout(std140, binding = 0) uniform MATERIAL {";
		for (auto uniform : m_UniformCache)
		{
			exp += Utils::ShaderUniformTypeToGlslType(uniform.Type);
			exp += " ";
			exp += uniform.Name;
			exp += ";";
		}
		exp += "}";
		Utils::SetShaderMacroExpression(source, macro, exp);
	}
}
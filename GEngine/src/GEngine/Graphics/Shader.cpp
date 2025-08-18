#include "GEpch.h"
#include "Shader.h"
#include "GEngine/Graphics/Graphics.h"
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
		ShaderPropertyType ShaderPropertyTypeFromString(const std::string& type)
		{
			if (ToLower(type) == VAR_NAME(int))					return ShaderPropertyType::Int;
			if (ToLower(type) == VAR_NAME(float))				return ShaderPropertyType::Float;
			if (ToLower(type) == VAR_NAME(vector))				return ShaderPropertyType::Vector;
			if (ToLower(type) == VAR_NAME(color))				return ShaderPropertyType::Color;
			if (ToLower(type) == VAR_NAME(mat3))				return ShaderPropertyType::Mat3;
			if (ToLower(type) == VAR_NAME(mat4))				return ShaderPropertyType::Mat4;
			if (ToLower(type) == VAR_NAME(sampler2d))			return ShaderPropertyType::Sampler2D;
			if (ToLower(type) == VAR_NAME(samplercube))			return ShaderPropertyType::SamplerCube;
			if (ToLower(type) == VAR_NAME(storageimage2d))		return ShaderPropertyType::StorageImage2D;
			if (ToLower(type) == VAR_NAME(storagebuffer))		return ShaderPropertyType::StorageBuffer;
			if (ToLower(type) == VAR_NAME(texture2d))			return ShaderPropertyType::Texture2D;
			if (ToLower(type) == VAR_NAME(sampler))				return ShaderPropertyType::Sampler;
			if (ToLower(type) == VAR_NAME(sampler2darray))		return ShaderPropertyType::Sampler2DArray;
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return ShaderPropertyType::None;
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
	}

	void Shader::Preprocess(const std::string& source, std::vector<std::vector<std::string>>& passStages, std::vector<std::string>& shaderSrcCode)
	{
		std::stack<int> stack;
		size_t pos = source.find("Shader", 0);
		pos = source.find("\"", pos);

		m_Name = source.substr(pos + 1, source.find("\"", pos + 1) - pos - 1);

		pos = source.find("{", pos);
		stack.push(1);

		pos = source.find("Properties", pos);
		pos = source.find("{", pos);
		stack.push(1);

		std::string block;
		block = source.substr(pos + 1, source.find("}", pos + 1) - pos - 1);

		//todo

		pos = source.find("}", pos);
		stack.pop();

		while (source.find("Pass", pos) != std::string::npos)
		{
			pos = source.find("Pass", pos);
			pos = source.find("{", pos);
			stack.push(1);

			block = source.substr(pos + 1, source.find("Program", pos + 1) - pos - 1);

			//todo

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
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
	std::string		const	ShaderMacroName::GE_MATERIAL_DEFINE				= "GE_MATERIAL_DEFINE";

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
				return "int";
			case ShaderUniformType::Float:
				return "float";
			case ShaderUniformType::Vector:
				return "vec4";
			case ShaderUniformType::Color:
				return "vec4";
			case ShaderUniformType::Mat3:
				return "mat3";
			case ShaderUniformType::Mat4:
				return "mat4";
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
			if (ToLower(flag) == "position")	return 0;
			if (ToLower(flag) == "color")		return 1;
			if (ToLower(flag) == "normal")		return 2;
			if (ToLower(flag) == "tangent")		return 3;
			if (ToLower(flag) == "uv0")			return 4;
			if (ToLower(flag) == "uv1")			return 5;
			if (ToLower(flag) == "uv2")			return 6;
			if (ToLower(flag) == "uv3")			return 7;
			GE_CORE_ASSERT(false, "invalid vertex input");
		}
		bool ShaderBoolFromString(const std::string& value)
		{
			if (ToLower(value) == "on")				return true;
			if (ToLower(value) == "1")				return true;
			if (ToLower(value) == "true")			return true;
			return false;
		}
		CompareOperation ShaderCompareOperationFromString(const std::string& value)
		{
			if (ToLower(value) == "less")				return CompareOperation::Less;
			if (ToLower(value) == "greater")			return CompareOperation::Greater;
			if (ToLower(value) == "lessequal")			return CompareOperation::LessEqual;
			if (ToLower(value) == "lequal")				return CompareOperation::LessEqual;
			if (ToLower(value) == "greaterequal")		return CompareOperation::GreaterEqual;
			if (ToLower(value) == "gequal")				return CompareOperation::GreaterEqual;
			if (ToLower(value) == "equal")				return CompareOperation::Equal;
			if (ToLower(value) == "notequal")			return CompareOperation::NotEqual;
			if (ToLower(value) == "always")				return CompareOperation::Always;

			return CompareOperation::LessEqual;
		}
		CullMode ShaderCullModeFromString(const std::string& value)
		{
			if (ToLower(value) == "none") return CullMode::None;
			if (ToLower(value) == "back") return CullMode::Back;
			if (ToLower(value) == "front") return CullMode::Front;
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
			if (ToLower(type) == "int")				return ShaderUniformType::Int;
			if (ToLower(type) == "float")			return ShaderUniformType::Float;
			if (ToLower(type) == "vector")			return ShaderUniformType::Vector;
			if (ToLower(type) == "color")			return ShaderUniformType::Color;
			if (ToLower(type) == "mat3")			return ShaderUniformType::Mat3;
			if (ToLower(type) == "mat4")			return ShaderUniformType::Mat4;
			if (ToLower(type) == "sampler2d")		return ShaderUniformType::Sampler2D;
			if (ToLower(type) == "samplercube")		return ShaderUniformType::SamplerCube;
			if (ToLower(type) == "storageimage2d")	return ShaderUniformType::StorageImage2D;
			if (ToLower(type) == "storagebuffer")	return ShaderUniformType::StorageBuffer;
			if (ToLower(type) == "texture2d")		return ShaderUniformType::Texture2D;
			if (ToLower(type) == "sampler")			return ShaderUniformType::Sampler;
			GE_CORE_ASSERT(false, "Unknown shader uniform type! " + type);
			return ShaderUniformType::None;
		}
		BlendFactor ShaderBlendFactorFromString(const std::string& factor)
		{
			if (ToUpper(factor) == "SRCALPHA")			return BlendFactor::SRC_ALPHA;
			if (ToUpper(factor) == "DSTALPHA")			return BlendFactor::DST_ALPHA;
			if (ToUpper(factor) == "SRCCOLOR")			return BlendFactor::SRC_COLOR;
			if (ToUpper(factor) == "DSTCOLOR")			return BlendFactor::DST_COLOR;
			if (ToUpper(factor) == "ONEMINUSSRCALPHA")	return BlendFactor::ONE_MINUS_SRC_ALPHA;
			if (ToUpper(factor) == "ONEMINUSDSTALPHA")	return BlendFactor::ONE_MINUS_DST_ALPHA;
			if (ToUpper(factor) == "ONEMINUSSRCCOLOR")	return BlendFactor::ONE_MINUS_SRC_COLOR;
			if (ToUpper(factor) == "ONEMINUSDSTCOLOR")	return BlendFactor::ONE_MINUS_DST_COLOR;
			if (ToUpper(factor) == "ONE")				return BlendFactor::ONE;
			if (ToUpper(factor) == "ZERO")				return BlendFactor::ZERO;

			GE_CORE_ASSERT(false, "Unknown blend factor! " + factor);
		}
		BlendMode ShaderBlendModeFromString(const std::string& type)
		{
			if (ToLower(type) == "none")				return BlendMode::None;
			if (ToLower(type) == "add")					return BlendMode::Add;
			if (ToLower(type) == "substract")			return BlendMode::Substract;
			if (ToLower(type) == "reversesubstract")	return BlendMode::ReverseSubstract;
			if (ToLower(type) == "min")					return BlendMode::Min;
			if (ToLower(type) == "max")					return BlendMode::Max;

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
		exp += "};";
		Utils::SetShaderMacroExpression(source, macro, exp);
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
#include "GEpch.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/ShaderContainer.h"
#include "GEngine/Asset/GAsset.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Tools/FileSystemHelper.h"
#include "GEngine/Tools/Serializer.h"
#include "GEngine/Tools/OpenSSLTool.h"
#include "GEngine/Tools/ShaderCompiler.h"
#include "GEngine/Application.h"
#include <filesystem>
#include <regex>
#include <set>
#include <stdexcept>

namespace GEngine
{
	namespace
	{
		// Runtime-scoped, render-thread-only cache. Names remain compatibility aliases.
		std::unordered_map<std::string, Ref<Shader>> s_ShaderPaths;
	}

	std::unordered_map<std::string, Ref<Shader>> Shader::s_Shaders = std::unordered_map<std::string, Ref<Shader>>();

	namespace
	{
		std::string ShaderCacheSource(const std::string& source)
		{
            static const std::regex includePattern(R"inc((?:^|\n)[ \t]*#[ \t]*include\s*["<]([^">]+)[">])inc");
            static const std::regex comments(R"inc(/\*[\s\S]*?\*/|//[^\r\n]*)inc");
            std::set<std::string> visited;
            std::string result = source + "\nGEngine-ShaderContainer:2";
            std::function<void(const std::string&)> append = [&](const std::string& text)
            {
                const auto active = std::regex_replace(text, comments, "");
                for (std::sregex_iterator match(active.begin(), active.end(), includePattern), end; match != end; ++match)
                {
                    const auto path = std::filesystem::path((*match)[1].str()).lexically_normal().generic_string();
                    if (!visited.insert(path).second) continue;
                    const auto included = std::filesystem::path(path).extension() == ".gasset"
                        ? LoadGAssetText(path, "shader-include")
                        : FileSystemHelper::ReadFileAsString(path);
                    result += "\nGEngine-Include:" + path + "\n" + included;
                    append(included);
                }
            };
            append(source); return result;
        }
		std::string GetSPIRVCacheSource(const std::string& source)
		{
			const bool isOpenGL = Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL;
			const char* target = isOpenGL ? "vulkan1.3;reflect=true;glsl450-cross-v1" : "vulkan1.3;reflect=true";
			return ShaderCacheSource(source) + "\nGEngine-SPIRV-Target:" + target;
		}
	}

	void Shader::ShutdownCache()
	{
		s_ShaderPaths.clear();
		s_Shaders.clear();
	}

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
		std::vector<std::string>												srcCodes; // each pass src code
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>	shaders; // pass { stage : code }
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>	reflectionDxils; // pass { stage : reflection dxil }
		const bool isGAsset = std::filesystem::path(path).extension() == ".gasset";
		std::string																source = isGAsset
			? LoadGAssetText(path, "shader")
			: FileSystemHelper::ReadFileAsString(path);
		ShaderCacheInfo															cache;
		Preprocess(source, srcCodes);
		FileSystemHelper::CreateFolder(Application::Get().GetConfig()->GetShaderCacheDirectory());
		if (LoadFromCache(shaders, reflectionDxils, source, cache))
		{
			ReflectShader(shaders, reflectionDxils);
			processMachingCodeFunc(shaders);
			GE_INFO("Load shader {} from cache.", m_Name);
			return;
		}
		if (!Compile(srcCodes, shaders, reflectionDxils))
			throw std::runtime_error("Shader compilation failed: " + path);
		ReflectShader(shaders, reflectionDxils);
		ComputeShaderCacheInfo(source, cache);
		SaveToCache(shaders, reflectionDxils, cache);
		processMachingCodeFunc(shaders);
	}

	bool Shader::IdentifyShaderCache(const std::string& source, ShaderCacheInfo& cache)
	{
		if(Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL ||
			Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
		{
			std::string hashCode = OpenSSLTool::ComputeMD5(GetSPIRVCacheSource(source));
			return cache.Name == m_Name && cache.SpirvHash == hashCode;
		}
		std::string hashCode = OpenSSLTool::ComputeMD5(ShaderCacheSource(source));
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
			return cache.Name == m_Name && cache.DxilHash == hashCode;
		// this should never happen, but just in case
		return cache.Name == m_Name && cache.SpirvHash == hashCode && cache.DxilHash == hashCode;
	}

	void Shader::Preprocess(const std::string& source, std::vector<std::string>& shaderSrcCode)
	{
		auto container = ParseShaderContainer(source, m_FilePath);
		std::vector<ShaderReflectionInfo> reflections;
		std::vector<std::unordered_map<std::string, std::string>> entries;
		std::vector<uint8_t> mesh;
		std::vector<std::string> programs;
		for (auto& pass : container.Passes)
		{
			ShaderReflectionInfo reflection;
			reflection.State = std::move(pass.State);
			reflections.push_back(std::move(reflection));
			entries.push_back(std::move(pass.EntryPoints));
			mesh.push_back(pass.MeshWorkflow ? 1 : 0);
			programs.push_back(std::move(pass.Program));
		}
		// Commit only after the entire container and all temporary allocations succeed.
		m_Name.swap(container.Name);
		m_PassReflections.swap(reflections);
		m_StageEntryPoints.swap(entries);
		m_IsMeshWorkflow.swap(mesh);
		shaderSrcCode.swap(programs);
	}

	bool Shader::Compile(const std::vector<std::string>& shaderSrcCodes, 
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders,
		std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionDxils)
	{
		if (m_StageEntryPoints.empty() || m_StageEntryPoints.size() != m_PassReflections.size())
			throw std::runtime_error("Shader has no valid passes or entry points: " + m_FilePath);
		for (int i = 0; i < m_StageEntryPoints.size(); i++)
		{
			if (i >= shaderSrcCodes.size() || m_StageEntryPoints.at(i).empty())
				throw std::runtime_error("Shader pass has no source or entry points: " + m_FilePath);
			shaders.push_back(std::unordered_map<std::string, std::vector<std::byte>>());
			if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL ||
				Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN ||
				Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
			{
				reflectionDxils.push_back(std::unordered_map<std::string, std::vector<std::byte>>());
			}
			
			
			for (auto&& [stage, entryPoint] : m_StageEntryPoints.at(i))
			{
				// 反射所有阶段资源的并集
				if(Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL ||
					Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN ||
					Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
				{
					std::vector<std::byte> machineCode;
					std::vector<std::byte> reflectionDxil;
					bool result = ShaderCompiler::Get()->Compile(shaderSrcCodes.at(i), stage, entryPoint, machineCode, reflectionDxil);
					if (!result || machineCode.empty() || reflectionDxil.empty())
						throw std::runtime_error("Failed to compile shader stage " + stage + " for pass " + std::to_string(i) + ": " + m_FilePath);
					shaders.at(i)[stage] = machineCode;
					reflectionDxils.at(i)[stage] = reflectionDxil;
				}
				
			}
		}
		
		
		return true;
	}

	void Shader::ComputeShaderCacheInfo(const std::string& source, ShaderCacheInfo& cache)
	{
		if(Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL ||
			Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
		{
			cache.SpirvHash = OpenSSLTool::ComputeMD5(GetSPIRVCacheSource(source));
		}
		if(Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
			cache.DxilHash = OpenSSLTool::ComputeMD5(ShaderCacheSource(source));
		cache.Name = m_Name;
	}

	void Shader::SaveToCache(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders, 
								const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionDxils,
								ShaderCacheInfo& cache)
	{
		bool res = false;
		std::string machineCodeExt;
		std::string cacheFolder = Application::Get().GetConfig()->GetShaderCacheDirectory() + FileSystemHelper::GetDocumentNameWithoutExtension(m_FilePath);
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL: 
		case GRAPHICS_API_VULKAN: machineCodeExt = SHADER_CACHE_FILE_EXTENSION_SPIRV; break;
		case GRAPHICS_API_DIRECT3DX12: machineCodeExt = SHADER_CACHE_FILE_EXTENSION_DXIL; break;
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
				std::string cachePath = cachePathSrc + "." + stage + machineCodeExt;
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
		// each pass and each stage reflection dxil write in sperate file
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL ||
			Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN ||
			Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
		{
			// spirv reflection need dxil reflection meta, so we need to save dxil reflection meta to cache for vulkan, too.
			for (int i = 0; i < reflectionDxils.size(); i++)
			{
				std::string cachePathSrc = cacheFolder + "/" + std::to_string(i);
				for (auto& [stage, meta] : reflectionDxils.at(i))
				{
					// create document
					std::string cachePath = cachePathSrc + "." + stage + SHADER_CACHE_REFLECTION_FILE_EXTENSION_DXIL;
					res = FileSystemHelper::CreateDocument(cachePath);
					res = FileSystemHelper::IsDocument(cachePath);
					GE_CORE_ASSERT(res, "Failed to create shader reflection dxil cache file!");
					// write reflection dxil
					std::vector<char> data(meta.size());
					memcpy(data.data(), meta.data(), meta.size());
					res = FileSystemHelper::WriteFile(cachePath, data);
					GE_CORE_ASSERT(res, "Failed to write shader reflection dxil cache file!");
				}
			}
		}
	}

	bool Shader::LoadFromCache(std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders, 
								std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionDxils,
								const std::string& source, ShaderCacheInfo& cache)
	{
		bool res = false;
		std::string machineCodeExt;
		std::string cacheFolder = Application::Get().GetConfig()->GetShaderCacheDirectory() + FileSystemHelper::GetDocumentNameWithoutExtension(m_FilePath);
		switch (Graphics::GetGraphicsAPI())
		{
		case GRAPHICS_API_OPENGL: 
		case GRAPHICS_API_VULKAN: machineCodeExt = SHADER_CACHE_FILE_EXTENSION_SPIRV; break;
		case GRAPHICS_API_DIRECT3DX12: machineCodeExt = SHADER_CACHE_FILE_EXTENSION_DXIL; break;
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
		cache = cacheInfo;
		if (IdentifyShaderCache(source, cacheInfo) == false)
		{
			GE_CORE_INFO("Shader cache is invalid, shader source may be changed, recompile shader! Cache name: {}", cacheInfo.Name);
			return false;
		}
		// process cache, each pass and each stage read from sperate file, the file name should be in format of "pass.stage.ext", for example "0.vertex.spirv"
		std::vector<std::string> cacheShaderFiles = FileSystemHelper::GetDocumentsInFolder(cacheFolder, machineCodeExt, "");
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
		if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL 
			|| Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN 
			|| Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
		{
			// spirv reflection need dxil reflection meta, so we need to load dxil reflection meta from cache
			std::vector<std::string> cacheReflectionDxilFiles = FileSystemHelper::GetDocumentsInFolder(cacheFolder, SHADER_CACHE_REFLECTION_FILE_EXTENSION_DXIL, "");
			reflectionDxils.resize(passIndices.back() + 1);
			for (auto& file : cacheReflectionDxilFiles)
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
				reflectionDxils.at(passIndex)[stage] = meta;
				GE_CORE_INFO("Load shader reflection dxil cache file: {}, pass {}, stage {}!", file, passIndex, stage);
			}
		}
		return true;
	}

	void Shader::ReflectShader(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders,
								const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& reflectionDxils)
	{
		//reflection meta
		for (int i = 0; i < reflectionDxils.size(); i++)
		{
			for (auto&& [stage, data] : reflectionDxils.at(i))
			{
				if (Graphics::GetGraphicsAPI() == GRAPHICS_API_OPENGL ||
					Graphics::GetGraphicsAPI() == GRAPHICS_API_VULKAN)
				{
					ShaderCompiler::Get()->ReflectSpirv(shaders.at(i).at(stage),
											data, stage, m_PassReflections.at(i));
				}
				else if (Graphics::GetGraphicsAPI() == GRAPHICS_API_DIRECT3DX12)
				{
					ShaderCompiler::Get()->ReflectDxil(data, stage, m_PassReflections.at(i));
				}
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
			for (auto& input : m_PassReflections.at(i).VertexInputs)
			{
				// if the semantic name contains "TEXCOORD" and the semantic index is greater than or equal to 8, 
				// it is considered as instance input, otherwise it is considered as vertex input
				// such as TEXCOORD0, TEXCOORD1, TEXCOORD2, TEXCOORD3, TEXCOORD4, TEXCOORD5, TEXCOORD6, TEXCOORD7 are vertex inputs
				// TEXCOORD8, TEXCOORD9, TEXCOORD10, TEXCOORD11, TEXCOORD12, TEXCOORD13, TEXCOORD14, TEXCOORD15 are instance inputs
				if (input.SemanticName.find("TEXCOORD") != std::string::npos && input.SemanticIndex >= 8)
				{
					input.IsPerInstance = true;
					input.Offset = instanceInputStride;
					instanceInputStride += Utils::ShaderInputDataSize(input.Type);
				}
				else
				{
					input.IsPerInstance = false;
					input.Offset = vertexInputStride;
					vertexInputStride += Utils::ShaderInputDataSize(input.Type);
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
		const auto api = Graphics::GetGraphicsAPI();
		const auto key = std::to_string(static_cast<int>(api)) + ":" +
			std::filesystem::absolute(path).lexically_normal().generic_string();
		const auto cached = s_ShaderPaths.find(key);
		if (cached != s_ShaderPaths.end())
			return cached->second;

		auto shader = Graphics::GetRenderDevice().CreateShader(path);
		if (!shader)
		{
			GE_CORE_ERROR("Cannot create shader: unsupported graphics API {}", static_cast<int>(api));
			return nullptr;
		}
		// Preserve the existing first-loaded-name behavior for distinct source files.
		auto named = GetShader(shader->GetShaderName());
		if (named) shader = named;
		else s_Shaders.emplace(shader->GetShaderName(), shader);
		s_ShaderPaths.emplace(key, shader);
		return shader;
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

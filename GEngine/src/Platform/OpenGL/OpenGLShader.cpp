#include "GEpch.h"
#include "OpenGLShader.h"
#include "GEngine/Renderer/Material.h"
#include <glad/glad.h>
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
	namespace Utils {
		
		

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (ToLower(type) == ShaderStage::Vertex)
				return GL_VERTEX_SHADER;
			if (ToLower(type) == ShaderStage::Fragment || ToLower(type) == ShaderStage::Pixel)
				return GL_FRAGMENT_SHADER;
			if (ToLower(type) == ShaderStage::Compute)
				return GL_COMPUTE_SHADER;
			if (ToLower(type) == ShaderStage::TessellationControl)
				return GL_TESS_CONTROL_SHADER;
			if (ToLower(type) == ShaderStage::TessellationEvaluation)
				return GL_TESS_EVALUATION_SHADER;
			if (ToLower(type) == ShaderStage::Geometry)
				return GL_GEOMETRY_SHADER;

			GE_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static std::string StringFromShaderType(GLenum type)
		{
			switch (type)
			{
			case GL_VERTEX_SHADER:			return ShaderStage::Vertex;
			case GL_FRAGMENT_SHADER:		return ShaderStage::Fragment;
			case GL_COMPUTE_SHADER:			return ShaderStage::Compute;
			case GL_TESS_CONTROL_SHADER:	return ShaderStage::TessellationControl;
			case GL_TESS_EVALUATION_SHADER:	return ShaderStage::TessellationEvaluation;
			case GL_GEOMETRY_SHADER:		return ShaderStage::Geometry;
			}
			GE_CORE_ASSERT(false, "Unknown shader type");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:			return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER:		return shaderc_glsl_fragment_shader;
			case GL_COMPUTE_SHADER:			return shaderc_glsl_compute_shader;
			case GL_TESS_CONTROL_SHADER:	return shaderc_glsl_tess_control_shader;
			case GL_TESS_EVALUATION_SHADER:	return shaderc_glsl_tess_evaluation_shader;
			case GL_GEOMETRY_SHADER:		return shaderc_glsl_geometry_shader;
			}
			GE_CORE_ASSERT(false,"");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:			return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:		return ".cached_opengl.frag";
			case GL_COMPUTE_SHADER:			return ".cached_opengl.comp";
			case GL_TESS_CONTROL_SHADER:	return ".cached_opengl.tesc";
			case GL_TESS_EVALUATION_SHADER:	return ".cached_opengl_tese";
			case GL_GEOMETRY_SHADER:		return ".cached_opengl_geom";
			}
			GE_CORE_ASSERT(false, "");
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:			return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER:		return ".cached_vulkan.frag";
			case GL_COMPUTE_SHADER:			return ".cached_vulkan.comp";
			case GL_TESS_CONTROL_SHADER:	return ".cached_vulkan.tesc";
			case GL_TESS_EVALUATION_SHADER:	return ".cached_vulkan_tese";
			case GL_GEOMETRY_SHADER:		return ".cached_vulkan_geom";
			}
			GE_CORE_ASSERT(false, "");
			return "";
		}
	}

	

	OpenGLShader::OpenGLShader(const std::string& path)
	{
		
		m_FilePath = path;
		Utils::CreateCacheDirectoryIfNeeded();

		std::string src = Utils::ReadFile(path);
		auto shaderSources = PreProcess(src);
		//Compile(shaderSources);
		//CompileOrGetVulkanBinaries(shaderSources);
		CompileOrGetOpenGLBinaries(shaderSources);
		CreateProgram();
	}
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		//

		//m_Name = name;

		//// Create an empty vertex shader handle
		//GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		//// Send the vertex shader source code to GL
		//// Note that std::string's .c_str is NULL character terminated.
		//const GLchar* source = (const GLchar*)vertexSrc.c_str();
		//glShaderSource(vertexShader, 1, &source, 0);

		//// Compile the vertex shader
		//glCompileShader(vertexShader);

		//GLint isCompiled = 0;
		//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		//if (isCompiled == GL_FALSE)
		//{
		//	GLint maxLength = 0;
		//	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		//	// The maxLength includes the NULL character
		//	std::vector<GLchar> infoLog(maxLength);
		//	glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

		//	// We don't need the shader anymore.
		//	glDeleteShader(vertexShader);

		//	// Use the infoLog as you see fit.

		//	// In this simple program, we'll just leave
		//	GE_CORE_ERROR("Vertex Shader compilation failed");
		//	GE_CORE_ERROR("{0}", infoLog.data());

		//	GE_CORE_ASSERT(false, "Shader compilation failed");
		//	return;
		//}

		//// Create an empty fragment shader handle
		//GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//// Send the fragment shader source code to GL
		//// Note that std::string's .c_str is NULL character terminated.
		//source = (const GLchar*)fragmentSrc.c_str();
		//glShaderSource(fragmentShader, 1, &source, 0);

		//// Compile the fragment shader
		//glCompileShader(fragmentShader);

		//glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		//if (isCompiled == GL_FALSE)
		//{
		//	GLint maxLength = 0;
		//	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		//	// The maxLength includes the NULL character
		//	std::vector<GLchar> infoLog(maxLength);
		//	glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

		//	// We don't need the shader anymore.
		//	glDeleteShader(fragmentShader);
		//	// Either of them. Don't leak shaders.
		//	glDeleteShader(vertexShader);

		//	// Use the infoLog as you see fit.

		//	// In this simple program, we'll just leave

		//	GE_CORE_ERROR("Fragment Shader compilation failed");
		//	GE_CORE_ERROR("{0}", infoLog.data());

		//	GE_CORE_ASSERT(false, "Shader compilation failed");
		//	return;
		//}

		//// Vertex and fragment shaders are successfully compiled.
		//// Now time to link them together into a program.
		//// Get a program object.
		//m_RendererID = glCreateProgram();
		//GLuint program = m_RendererID;

		//// Attach our shaders to our program
		//glAttachShader(program, vertexShader);
		//glAttachShader(program, fragmentShader);

		//// Link our program
		//glLinkProgram(program);

		//// Note the different functions here: glGetProgram* instead of glGetShader*.
		//GLint isLinked = 0;
		//glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		//if (isLinked == GL_FALSE)
		//{
		//	GLint maxLength = 0;
		//	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//	// The maxLength includes the NULL character
		//	std::vector<GLchar> infoLog(maxLength);
		//	glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		//	// We don't need the program anymore.
		//	glDeleteProgram(program);
		//	// Don't leak shaders either.
		//	glDeleteShader(vertexShader);
		//	glDeleteShader(fragmentShader);

		//	// Use the infoLog as you see fit.

		//	GE_CORE_ERROR("Shader link failed");
		//	GE_CORE_ERROR("{0}", infoLog.data());

		//	GE_CORE_ASSERT(false, "Shader link failed");
		//	return;
		//}

		//// Always detach shaders after a successful link.
		//glDetachShader(program, vertexShader);
		//glDetachShader(program, fragmentShader);

		
	}
	OpenGLShader::~OpenGLShader()
	{
			glDeleteProgram(m_RendererID);
	}
	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}
	void OpenGLShader::SetInt1(const std::string& name, int value)
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
	}
	void OpenGLShader::SetIntArray(const std::string& name, int* value, uint32_t count)
	{
		glUniform1iv(glGetUniformLocation(m_RendererID, name.c_str()), count, value);
	}
	void OpenGLShader::SetFloat1(const std::string& name, float value)
	{
		glUniform1f(glGetUniformLocation(m_RendererID, name.c_str()), value);
	}
	void OpenGLShader::SetFloat2(const std::string& name, const Vector2& value)
	{
		glUniform2f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y);
	}
	void OpenGLShader::SetFloat3(const std::string& name, const Vector3& value)
	{
		glUniform3f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y, value.z);
	}
	void OpenGLShader::SetFloat4(const std::string& name, const Vector4& value)
	{
		glUniform4f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y, value.z, value.w);
	}
	void OpenGLShader::SetMat4x4(const std::string& name, const Matrix4x4& value)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name.c_str()), 1, GL_FALSE, Math::ValuePtr(value));
	}
	void OpenGLShader::SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name.c_str()), count, GL_FALSE, Math::ValuePtr(*value));
	}
	void OpenGLShader::SetUniformMat4(const std::string& name, const Matrix4x4& matrix)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name.c_str()), 1, GL_FALSE, Math::ValuePtr(matrix));
	}
	void OpenGLShader::SetUniformInt(const std::string& name, int value)
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
	}
	void OpenGLShader::SetUniformFloat(const std::string& name, float value)
	{
		glUniform1f(glGetUniformLocation(m_RendererID, name.c_str()), value);
	}
	void OpenGLShader::SetUniformFloat4(const std::string& name, const Vector4& vector)
	{
		glUniform4f(glGetUniformLocation(m_RendererID, name.c_str()), vector.x, vector.y, vector.z, vector.w);
	}
	void OpenGLShader::SetUniformTexture2D(const std::string& name, int slot)
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), slot);
	}
	void OpenGLShader::SetMacroBool(std::string& source)
	{
		for (int i = 0; i < m_MacroBools.size(); i++)
		{
			Utils::SetShaderMacroBool(source, m_MacroBools[i].first, m_MacroBools[i].second);
		}
	}
	void OpenGLShader::SetMacroExp(std::string& source)
	{
		for (int i = 0; i < m_MacroExps.size(); i++)
		{
			Utils::SetShaderMacroExpression(source, m_MacroExps[i].first, m_MacroExps[i].second);
		}
	}
	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* propertyToken		= "#Properties";
		const char* propertyEndToken	= "#EndProperties";
		size_t propertyTokenLength		= strlen(propertyToken);

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
			for(auto it = props.begin(); it != props.end();)
			{
				if (it->empty())
					it = props.erase(it);
				else
					++it;
			}
			uint32_t location = 0;
			uint32_t slot = 0;
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
					
					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property slot: {2}", uniform.Name, propType, slot);

					slot++;
					m_Texture2DCache.push_back(uniform);
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::SamplerCube)
				{
					ShaderUniformCubeMap	uniform;
					uniform.Name			= propName;
					uniform.Slot			= slot;
					uniform.Cubemap			= CubeMap::White();

					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property slot: {2}", uniform.Name, propType, slot);

					slot++;
					m_CubeMapCache.push_back(uniform);
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::StorageImage2D)
				{
					ShaderUniformStorageImage2D	uniform;
					uniform.Name				= propName;
					uniform.Slot				= slot;
					uniform.Image				= nullptr;

					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property slot: {2}", uniform.Name, propType, slot);

					slot++;
					m_StorageImage2DCache.push_back(uniform);
				}
				if (Utils::ShaderUniformTypeFromString(propType) == ShaderUniformType::StorageBuffer)
				{
					ShaderUniformStorageBuffer	uniform;
					uniform.Name				= propName;
					uniform.Slot				= slot + Shader::s_SlotOffset;
					uniform.Buffer				= nullptr;

					GE_CORE_TRACE("Property Name: {0}, Property Type: {1}, Property binding: {2}", uniform.Name, propType, uniform.Slot);

					slot++;
					m_StorageBufferCache.push_back(uniform);
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
			GE_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}
		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

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
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
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

		/*for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);*/
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries(std::unordered_map<GLenum, std::string>& shaderSources)
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetIncluder(std::make_unique<ShaderIncluder>());
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
				SetMacroBool(source);
				SetMacroExp(source);
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
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

		/*for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);*/
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, m_ShaderMainFuncName.c_str(), 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}
		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			GE_CORE_ASSERT(false, "Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
		m_RendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources				= compiler.get_shader_resources();

		GE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::StringFromShaderType(stage), m_FilePath);
		GE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GE_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		GE_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType	= compiler.get_type(resource.base_type_id);
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
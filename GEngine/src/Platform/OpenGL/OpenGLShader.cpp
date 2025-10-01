#include "GEpch.h"
#include "OpenGLShader.h"
#include "GEngine/Graphics/Material.h"
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
		

		GLenum ShaderStageToGL(const std::string& stage)
		{
			if (stage == SHADER_STAGE_VERTEX)					return GL_VERTEX_SHADER;
			if (stage == SHADER_STAGE_FRAGMENT)					return GL_FRAGMENT_SHADER;
			if (stage == SHADER_STAGE_COMPUTE)					return GL_COMPUTE_SHADER;
			if (stage == SHADER_STAGE_HULL)						return GL_TESS_CONTROL_SHADER;
			if (stage == SHADER_STAGE_DOMAIN)					return GL_TESS_EVALUATION_SHADER;
			if (stage == SHADER_STAGE_GEOMETRY)					return GL_GEOMETRY_SHADER;
			if (stage == SHADER_STAGE_AMPLIFICATION)			return GL_TASK_SHADER_NV;
			if (stage == SHADER_STAGE_MESH)						return GL_MESH_SHADER_NV;
			GE_CORE_ASSERT(false, "Unknown shader stage!");
			return 0;
		}

		std::string GLToShaderStage(GLenum type)
		{
			switch (type)
			{
			case GL_VERTEX_SHADER:			return SHADER_STAGE_VERTEX;
			case GL_FRAGMENT_SHADER:		return SHADER_STAGE_FRAGMENT;
			case GL_COMPUTE_SHADER:			return SHADER_STAGE_COMPUTE;
			case GL_TESS_CONTROL_SHADER:	return SHADER_STAGE_HULL;
			case GL_TESS_EVALUATION_SHADER:	return SHADER_STAGE_DOMAIN;
			case GL_GEOMETRY_SHADER:		return SHADER_STAGE_GEOMETRY;
			case GL_TASK_SHADER_NV:			return SHADER_STAGE_AMPLIFICATION;
			case GL_MESH_SHADER_NV:			return SHADER_STAGE_MESH;
			}
			GE_CORE_ASSERT(false, "Unknown shader type");
			return 0;
		}

		shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:			return shaderc_vertex_shader;
			case GL_FRAGMENT_SHADER:		return shaderc_fragment_shader;
			case GL_COMPUTE_SHADER:			return shaderc_compute_shader;
			case GL_TESS_CONTROL_SHADER:	return shaderc_tess_control_shader;
			case GL_TESS_EVALUATION_SHADER:	return shaderc_tess_evaluation_shader;
			case GL_GEOMETRY_SHADER:		return shaderc_geometry_shader;
			case GL_TASK_SHADER_NV:			return shaderc_task_shader;
			case GL_MESH_SHADER_NV:			return shaderc_mesh_shader;
			}
			GE_CORE_ASSERT(false,"");
			return (shaderc_shader_kind)0;
		}

		const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:			return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:		return ".cached_opengl.frag";
			case GL_COMPUTE_SHADER:			return ".cached_opengl.comp";
			case GL_TESS_CONTROL_SHADER:	return ".cached_opengl.hull";
			case GL_TESS_EVALUATION_SHADER:	return ".cached_opengl_doma";
			case GL_GEOMETRY_SHADER:		return ".cached_opengl_geom";
			case GL_TASK_SHADER_NV:			return ".cached_opengl_ampl";
			case GL_MESH_SHADER_NV:			return ".cached_opengl_mesh";
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
		ProcessShaderSource(src);
		for (auto pass : m_ShaderPasses)
		{
			auto spirvData = CompileOpenGLBinaries(pass);
			uint32_t id = CreateProgram(spirvData);
			m_Shaders[pass.first] = id;
		}
		m_ShaderBlocks.clear();
		m_ShaderPasses.clear();
	}
	OpenGLShader::~OpenGLShader()
	{
		
		for(auto shader : m_Programs)
			glDeleteProgram(shader);
	}
	
	void OpenGLShader::Use(const int& pass)
	{
		glUseProgram(m_Programs[pass]);
	}
	void OpenGLShader::SetInt1(const std::string& name, int value, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniform1i(glGetUniformLocation(m_Programs[pass], name.c_str()), value);
	}
	void OpenGLShader::SetIntArray(const std::string& name, int* value, uint32_t count, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniform1iv(glGetUniformLocation(m_Programs[pass], name.c_str()), count, value);
	}
	void OpenGLShader::SetFloat1(const std::string& name, float value, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniform1f(glGetUniformLocation(m_Programs[pass], name.c_str()), value);
	}
	void OpenGLShader::SetFloat2(const std::string& name, const Vector2& value, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniform2f(glGetUniformLocation(m_Programs[pass], name.c_str()), value.x, value.y);
	}
	void OpenGLShader::SetFloat3(const std::string& name, const Vector3& value, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniform3f(glGetUniformLocation(m_Programs[pass], name.c_str()), value.x, value.y, value.z);
	}
	void OpenGLShader::SetFloat4(const std::string& name, const Vector4& value, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniform4f(glGetUniformLocation(m_Programs[pass], name.c_str()), value.x, value.y, value.z, value.w);
	}
	void OpenGLShader::SetMat4x4(const std::string& name, const Matrix4x4& value, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniformMatrix4fv(glGetUniformLocation(m_Programs[pass], name.c_str()), 1, GL_FALSE, Math::ValuePtr(value));
	}
	void OpenGLShader::SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count, const int& pass)
	{
		GE_CORE_ASSERT(m_Programs.size() > pass, "pass {} not found", pass);
		glUniformMatrix4fv(glGetUniformLocation(m_Programs[pass], name.c_str()), count, GL_FALSE, Math::ValuePtr(*value));
	}
	std::unordered_map<std::string, std::string> OpenGLShader::ProcessShaderSource(const std::string& source)
	{
		std::unordered_map<std::string, std::string> shaderSources;

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
		Utils::ProcessShaderProperties(source, m_UniformCache, m_Texture2DCache, m_CubeMapCache, m_StorageImage2DCache, m_StorageBufferCache, 0, 0, Shader::s_SlotOffset);

		// find Type
		
		size_t pos = source.find(typeToken, 0);

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

		// multi pass
		Utils::ProcessShaderBlocks(source, m_ShaderBlocks);
		Utils::ProcessShaderPasses(source, m_ShaderBlocks, m_ShaderPasses, m_RenderStates);
	}

	std::unordered_map<std::string, std::vector<uint32_t>> OpenGLShader::CompileOpenGLBinaries(std::pair<std::string, ShaderPass> pass)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetIncluder(std::make_unique<ShaderIncluder>());
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		std::unordered_map<std::string, std::vector<uint32_t>> shaderData;
		for (auto&& [stage, source] : pass.second.Stages)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + "." + pass.first + Utils::GLShaderStageCachedOpenGLFileExtension(Utils::ShaderStageToGL(stage)));

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
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(Utils::ShaderStageToGL(stage)), m_FilePath.c_str(), options);
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

	uint32_t OpenGLShader::CreateProgram(std::unordered_map<std::string, std::vector<uint32_t>> shader)
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : shader)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(Utils::ShaderStageToGL(stage)));
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
		return program;
	}


	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources				= compiler.get_shader_resources();

		GE_CORE_INFO("OpenGLShader::Reflect - {0} {1}", Utils::GLToShaderStage(stage), m_FilePath);
		GE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GE_CORE_TRACE("    {0} sampled images", resources.sampled_images.size());
		GE_CORE_TRACE("    {0} separate images", resources.separate_images.size());
		GE_CORE_TRACE("    {0} separate samplers", resources.separate_samplers.size());

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
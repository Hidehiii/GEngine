#include "GEpch.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "GEngine/Graphics/Material.h"
#include <glad/glad.h>
#include <SPIRVCross/spirv_glsl.hpp>
#include <stdexcept>

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
	}

	

	OpenGLShader::OpenGLShader(const std::string& path)
	{
		InitializeShader(path, GE_BIND_CLASS_FUNCTION_LAMBDA(ProcessMachineCode));
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

	void OpenGLShader::ProcessMachineCode(const std::vector<std::unordered_map<std::string, std::vector<std::byte>>>& shaders)
	{
		m_Programs.clear();
		m_Programs.resize(shaders.size());
		for (int i = 0; i < shaders.size(); i++)
		{
			GLuint program = glCreateProgram();
			std::vector<GLuint> shaderIDs;
			try
			{
			for (auto&& [stage, byte] : shaders[i])
			{
				if (byte.empty() || byte.size() % sizeof(uint32_t) != 0)
					throw std::runtime_error("Invalid SPIR-V intermediate: " + m_FilePath);
				std::vector<uint32_t> words(byte.size() / sizeof(uint32_t));
				memcpy(words.data(), byte.data(), byte.size());
				spirv_cross::CompilerGLSL compiler(std::move(words));
				auto options = compiler.get_common_options();
				options.version = 450;
				options.es = false;
				options.vulkan_semantics = false;
				compiler.set_common_options(options);
				// DXC emits Vulkan SPIR-V, not an OpenGL-specializable binary.
				const auto source = compiler.compile();
				const auto* sourceText = source.c_str();
				GLuint shaderID = shaderIDs.emplace_back(glCreateShader(Utils::ShaderStageToGL(stage)));
				glShaderSource(shaderID, 1, &sourceText, nullptr);
				glCompileShader(shaderID);
				GLint compiled = GL_FALSE;
				glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
				if (compiled != GL_TRUE)
				{
					GLint length = 0;
					glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
					std::vector<GLchar> log(std::max(length, 1), '\0');
					glGetShaderInfoLog(shaderID, static_cast<GLsizei>(log.size()), nullptr, log.data());
					throw std::runtime_error("OpenGL shader compile failed: " + m_FilePath + " / " + stage + ": " + log.data());
				}
				glAttachShader(program, shaderID);
			}
			glLinkProgram(program);

			GLint isLinked;
			glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
			if (isLinked == GL_FALSE)
			{
				GLint maxLength;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(std::max(maxLength, 1), '\0');
				glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
				throw std::runtime_error("OpenGL shader link failed: " + m_FilePath + ": " + infoLog.data());
			}
			}
			catch (...)
			{
				for (auto id : shaderIDs) glDeleteShader(id);
				glDeleteProgram(program);
				for (auto completed : m_Programs) if (completed) glDeleteProgram(completed);
				m_Programs.clear();
				throw;
			}
			for (auto id : shaderIDs)
			{
				glDetachShader(program, id);
				glDeleteShader(id);
			}

			m_Programs[i] = program;
		}
	}

}

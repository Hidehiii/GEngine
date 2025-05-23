#include "GEpch.h"
#include "OpenGLGraphicsAPI.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "OpenGLUtils.h"
#include "Platform/OpenGL/OpenGLCommandBuffer.h"
#include "OpenGLContext.h"

namespace GEngine
{
	OpenGLGraphicsAPI::OpenGLGraphicsAPI()
	{
		s_API = GraphicsAPI::API::OpenGL;

#ifdef GE_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
			{
				switch (severity)
				{
				case GL_DEBUG_SEVERITY_HIGH:
					GE_CORE_CRITICAL(message);
					break;
				case GL_DEBUG_SEVERITY_MEDIUM:
					GE_CORE_ERROR(message);
					break;
				case GL_DEBUG_SEVERITY_LOW:
					GE_CORE_WARN(message);
					break;
				case GL_DEBUG_SEVERITY_NOTIFICATION:
					GE_CORE_TRACE(message);
					break;
				}
			}, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_MULTISAMPLE);
	}
	OpenGLGraphicsAPI::~OpenGLGraphicsAPI()
	{
	}


	Ref<CommandBuffer> OpenGLGraphicsAPI::GetGraphicsCommandBuffer()
	{
		return OpenGLContext::Get()->GetCommandBuffer(CommandBufferType::Graphics);
	}
	Ref<CommandBuffer> OpenGLGraphicsAPI::GetComputeCommandBuffer()
	{
		return OpenGLContext::Get()->GetCommandBuffer(CommandBufferType::Compute);
	}
	std::vector<std::string> OpenGLGraphicsAPI::GetExtensions()
	{
		std::vector<std::string> ext;
		GLint numExtensions;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

		for (GLint i = 0; i < numExtensions; i++) {
			const char* currentExt = (const char*)glGetStringi(GL_EXTENSIONS, i);
			ext.push_back(currentExt);
		};
		return ext;
	}
	uint32_t OpenGLGraphicsAPI::GetMaxTextureSize()
	{
		int size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
		return size;
	}
	uint32_t OpenGLGraphicsAPI::GetMaxCombinedTextureCount()
	{
		int count;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &count);
		return count;
	}
	uint32_t OpenGLGraphicsAPI::GetMaxPerStageTextureCount()
	{
		int count;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &count);
		return count;
	}
	uint32_t OpenGLGraphicsAPI::GetMaxTextureArrayLayers()
	{
		GLint maxArrayTextureLayers;
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);
		return maxArrayTextureLayers;
	}
	uint32_t OpenGLGraphicsAPI::GetMinUniformBufferOffsetAlignment()
	{
		GLint aligment;
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &aligment);
		return aligment;
	}
	uint32_t OpenGLGraphicsAPI::GetMaxUniformBufferSize()
	{
		GLint size;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
		return size;
	}
	Vector3 OpenGLGraphicsAPI::GetMaxComputeWorkGroupCount()
	{
		int x, y, z;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &x);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &y);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &z);
		return Vector3(x, y, z);
	}
	Vector3 OpenGLGraphicsAPI::GetMaxComputeWorkGroupSize()
	{
		int x, y, z;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &x);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &y);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &z);
		return Vector3(x, y, z);
	}
	uint32_t OpenGLGraphicsAPI::GetMaxComputeWorkGroupInvocations()
	{
		int x;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 0, &x);
		return x;
	}
}
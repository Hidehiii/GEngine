#include "GEpch.h"
#include "OpenGLGraphicsAPI.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "OpenGLUtils.h"
#include "Platform/OpenGL/OpenGLCommandBuffer.h"
#include "OpenGLContext.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

namespace GEngine
{
	OpenGLGraphicsAPI::OpenGLGraphicsAPI()
	{
		s_API = GRAPHICS_API_OPENGL;

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
		return OpenGLContext::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_GRAPHICS);
	}
	Ref<CommandBuffer> OpenGLGraphicsAPI::GetComputeCommandBuffer()
	{
		return OpenGLContext::Get()->GetCommandBuffer(COMMAND_BUFFER_TYPE_COMPUTE);
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
	GraphicsCapabilities OpenGLGraphicsAPI::GetCapabilities() const
	{
		GraphicsCapabilities capabilities;
		capabilities.RenderPass = true;
		capabilities.FrameBuffer = true;
		capabilities.Texture2D = true;
		capabilities.Texture2DArray = true;
		capabilities.CubeMap = true;
		capabilities.Sampler = true;
		capabilities.UniformBuffer = true;
		capabilities.StorageBuffer = true;
		capabilities.StorageImage = true;
		capabilities.Compute = true;
		return capabilities;
	}
	Ref<RenderPass> OpenGLGraphicsAPI::CreateRenderPass(const RenderPassSpecification& spec)
	{
		return CreateRef<OpenGLRenderPass>(spec);
	}
	Ref<FrameBuffer> OpenGLGraphicsAPI::CreateFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height)
	{
		return CreateRef<OpenGLFrameBuffer>(renderPass, width, height);
	}
	Ref<FrameBuffer> OpenGLGraphicsAPI::ResizeFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height)
	{
		return CreateRef<OpenGLFrameBuffer>(buffer, width, height);
	}
	Ref<Texture2D> OpenGLGraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, RenderImage2DFormat format)
	{
		return CreateRef<OpenGLTexture2D>(width, height, format);
	}
	Ref<Texture2D> OpenGLGraphicsAPI::CreateTexture2D(const std::string& path)
	{
		return CreateRef<OpenGLTexture2D>(path);
	}
	Ref<Texture2D> OpenGLGraphicsAPI::CreateTexture2D(uint32_t width, uint32_t height, void* data, uint32_t size, RenderImage2DFormat format)
	{
		return CreateRef<OpenGLTexture2D>(width, height, data, size, format);
	}
	Ref<UniformBuffer> OpenGLGraphicsAPI::CreateUniformBuffer(uint32_t size, uint32_t count, bool autoSetDataDynamic)
	{
		return CreateRef<OpenGLUniformBuffer>(size, count, autoSetDataDynamic);
	}
	Ref<VertexBuffer> OpenGLGraphicsAPI::CreateVertexBuffer(uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		return CreateRef<OpenGLVertexBuffer>(size, sizeInstance, type);
	}
	Ref<VertexBuffer> OpenGLGraphicsAPI::CreateVertexBuffer(const void* vertices, uint32_t size, uint32_t sizeInstance, VertexTopology type)
	{
		return CreateRef<OpenGLVertexBuffer>(vertices, size, sizeInstance, type);
	}
	Ref<IndexBuffer> OpenGLGraphicsAPI::CreateIndexBuffer(const uint32_t* indices, uint32_t count)
	{
		return CreateRef<OpenGLIndexBuffer>(indices, count);
	}
	uint32_t OpenGLGraphicsAPI::GetMaxTexture2DSize()
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

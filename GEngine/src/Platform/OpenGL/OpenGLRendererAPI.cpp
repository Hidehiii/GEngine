#include "GEpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "OpenGLUtils.h"
#include "Platform/OpenGL/OpenGLCommandBuffer.h"
#include "OpenGLContext.h"

namespace GEngine
{
	void OpenGLRendererAPI::Init()
	{
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

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_MULTISAMPLE);
	}
	void OpenGLRendererAPI::Uninit()
	{
	}
	void OpenGLRendererAPI::SetClearColor(const Vector4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(1.0f);
		glClearStencil(0.0f);
	}
	void OpenGLRendererAPI::Clear()
	{
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	void OpenGLRendererAPI::DrawTriangles(uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawLines(uint32_t indexCount)
	{
		glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawPoints(uint32_t indexCount)
	{
		glDrawElements(GL_POINTS, indexCount, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawTrianglesInstance(uint32_t indexCount, uint32_t instanceCount)
	{
		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
	}
	void OpenGLRendererAPI::DrawTriangles(CommandBuffer* buffer, uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawLines(CommandBuffer* buffer, uint32_t indexCount)
	{
		glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawPoints(CommandBuffer* buffer, uint32_t indexCount)
	{
		glDrawElements(GL_POINTS, indexCount, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawTrianglesInstance(CommandBuffer* buffer, uint32_t indexCount, uint32_t instanceCount)
	{
		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
	}
	void OpenGLRendererAPI::EnableDepthWrite(bool enabled)
	{
		if (enabled)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
	void OpenGLRendererAPI::SetDepthTest(CompareOperation op)
	{
		switch (op)
		{
		case CompareOperation::Less:		glDepthFunc(GL_LESS); break;
		case CompareOperation::Greater:		glDepthFunc(GL_GREATER); break;
		case CompareOperation::LessEqual:	glDepthFunc(GL_LEQUAL); break;
		case CompareOperation::GreaterEqual:glDepthFunc(GL_GEQUAL); break;
		case CompareOperation::Equal:		glDepthFunc(GL_EQUAL); break;
		case CompareOperation::NotEqual:	glDepthFunc(GL_NOTEQUAL); break;
		case CompareOperation::Always:		glDepthFunc(GL_ALWAYS); break;
		default:
			break;
		}
	}
	void OpenGLRendererAPI::SetCull(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::None:	glDisable(GL_CULL_FACE); break;
		case CullMode::Back:	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); break;
		case CullMode::Front:	glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); break;
		default:GE_CORE_CRITICAL("Unknown cull mode!"); break;
		}
	}
	void OpenGLRendererAPI::SetBlend(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
	{
		GLenum colorMode = GL_FUNC_ADD, alphaMode = GL_FUNC_ADD;
		switch (modeColor)
		{
		case BlendMode::None:
			glDisable(GL_BLEND);
			return;
		case BlendMode::Add:
			colorMode = GL_FUNC_ADD;
			break;
		case BlendMode::Substract:
			colorMode = (GL_FUNC_SUBTRACT);
			break;
		case BlendMode::ReverseSubstract:
			colorMode = (GL_FUNC_REVERSE_SUBTRACT);
			break;
		case BlendMode::Min:
			colorMode = (GL_MIN);
			break;
		case BlendMode::Max:
			colorMode = (GL_MAX);
			break;
		default:
			break;
		}
		switch (modeAlpha)
		{
		case BlendMode::None:
			glDisable(GL_BLEND);
			return;
		case BlendMode::Add:
			alphaMode = GL_FUNC_ADD;
			break;
		case BlendMode::Substract:
			alphaMode = GL_FUNC_SUBTRACT;
			break;
		case BlendMode::ReverseSubstract:
			alphaMode = GL_FUNC_REVERSE_SUBTRACT;
			break;
		case BlendMode::Min:
			alphaMode = GL_MIN;
			break;
		case BlendMode::Max:
			alphaMode = GL_MAX;
			break;
		default:
			break;
		}
		glBlendEquationSeparate(colorMode, alphaMode);
		glBlendFuncSeparate(Utils::BlendFactorToGLBlendFactor(srcColor),
			Utils::BlendFactorToGLBlendFactor(dstColor),
			Utils::BlendFactorToGLBlendFactor(srcAlpha),
			Utils::BlendFactorToGLBlendFactor(dstAlpha));
	}
	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
	void OpenGLRendererAPI::SetPointSize(float size)
	{
		glPointSize(size);
	}
	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}
	CommandBuffer* OpenGLRendererAPI::BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Camera& camera)
	{
		auto cmd = (OpenGLCommandBuffer*)OpenGLContext::Get()->GetCommandBuffer();
		cmd->Begin(buffer, camera);
		return cmd;
	}
	CommandBuffer* OpenGLRendererAPI::BeginGraphicsCommand(Ref<FrameBuffer>& buffer, const Editor::EditorCamera& camera)
	{
		auto cmd = (OpenGLCommandBuffer*)OpenGLContext::Get()->GetCommandBuffer();
		cmd->Begin(buffer, camera);
		return cmd;
	}
	void OpenGLRendererAPI::EndGraphicsCommand(CommandBuffer* buffer)
	{
		GE_CORE_ASSERT(buffer->GetType() == CommandBufferType::Graphics, "could not call EndGraphicsCommand with commandBuffer without Graphics type.");
		((OpenGLCommandBuffer*)buffer)->End();
	}
	float OpenGLRendererAPI::GetTime()
	{
		return (float)glfwGetTime();
	}
	std::vector<std::string> OpenGLRendererAPI::GetExtensions()
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
	uint32_t OpenGLRendererAPI::GetMaxTextureSize()
	{
		int size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
		return size;
	}
	uint32_t OpenGLRendererAPI::GetMaxCombinedTextureCount()
	{
		int count;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &count);
		return count;
	}
	uint32_t OpenGLRendererAPI::GetMaxPerStageTextureCount()
	{
		int count;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &count);
		return count;
	}
	uint32_t OpenGLRendererAPI::GetMaxTextureArrayLayers()
	{
		GLint maxArrayTextureLayers;
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);
		return maxArrayTextureLayers;
	}
	uint32_t OpenGLRendererAPI::GetMinUniformBufferOffsetAlignment()
	{
		GLint aligment;
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &aligment);
		return aligment;
	}
	uint32_t OpenGLRendererAPI::GetMaxUniformBufferSize()
	{
		GLint size;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
		return size;
	}
	Vector3 OpenGLRendererAPI::GetMaxComputeWorkGroupCount()
	{
		int x, y, z;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &x);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &y);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &z);
		return Vector3(x, y, z);
	}
	Vector3 OpenGLRendererAPI::GetMaxComputeWorkGroupSize()
	{
		int x, y, z;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &x);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &y);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &z);
		return Vector3(x, y, z);
	}
	uint32_t OpenGLRendererAPI::GetMaxComputeWorkGroupInvocations()
	{
		int x;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 0, &x);
		return x;
	}
	void OpenGLRendererAPI::Compute(const uint32_t x, const uint32_t y, const uint32_t z)
	{
		glDispatchCompute(x, y, z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	void OpenGLRendererAPI::Compute(CommandBuffer* buffer, const uint32_t x, const uint32_t y, const uint32_t z)
	{
		glDispatchCompute(x, y, z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}
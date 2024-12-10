#include "GEpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "OpenGLUtils.h"

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
	}
	void OpenGLRendererAPI::Uninit()
	{
	}
	void OpenGLRendererAPI::SetClearColor(const Vector4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(1.0f);
	}
	void OpenGLRendererAPI::Clear()
	{
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void OpenGLRendererAPI::DrawTriangles(uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
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
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void OpenGLRendererAPI::EnableDepthWrite(bool enabled)
	{
		if (enabled)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
	void OpenGLRendererAPI::EnableDepthTest(bool enabled)
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
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
	float OpenGLRendererAPI::GetTime()
	{
		return (float)glfwGetTime();
	}
}
#include "GEpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
	}
	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void OpenGLRendererAPI::DrawTriangles(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, indexCount);
	}
	void OpenGLRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_POINTS, 0, indexCount);
	}
	void OpenGLRendererAPI::DrawTrianglesInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		vertexArray->Bind();
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, instanceCount);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void OpenGLRendererAPI::EnableDepthMask(bool enabled)
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
	void OpenGLRendererAPI::SetCull(Material_CullMode mode)
	{
		switch (mode)
		{
		case Material_CullMode::None:	glDisable(GL_CULL_FACE); break;
		case Material_CullMode::Back:	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); break;
		case Material_CullMode::Front:	glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); break;
		default:GE_CORE_CRITICAL("Unknown cull mode!"); break;
		}
	}
	void OpenGLRendererAPI::SetBlend(Material_BlendMode mode, uint32_t source, uint32_t dest)
	{
		switch (mode)
		{
		case Material_BlendMode::None:
			glDisable(GL_BLEND);
			break;
		case Material_BlendMode::Alpha:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case Material_BlendMode::Additive:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
			break;
		case Material_BlendMode::Multiply:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			break;
		case Material_BlendMode::Customized:
			glEnable(GL_BLEND);
			glBlendFunc(source, dest);
			break;
		default:
			break;
		}
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
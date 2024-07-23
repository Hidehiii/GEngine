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
	}
	void OpenGLRendererAPI::SetClearColor(const Vector4& color)
	{
		glClearColor(color.value.r, color.value.g, color.value.b, color.value.a);
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
	void OpenGLRendererAPI::SetDepthTest(bool enabled)
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	void OpenGLRendererAPI::SetBlend(bool enabled)
	{
		if (enabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	
	}
	void OpenGLRendererAPI::SetBlendFunction(uint32_t source, uint32_t destination)
	{
		glBlendFunc(source, destination);
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
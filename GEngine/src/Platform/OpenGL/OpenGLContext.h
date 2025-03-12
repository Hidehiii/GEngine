#pragma once

#include "GEngine/Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
	class OpenGLCommandBuffer;

	class GENGINE_API OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual ~OpenGLContext() override;

		virtual void Init(const unsigned int width, const unsigned int height) override;
		virtual void Uninit() override;
		virtual void SwapBuffers() override;
		virtual void SetVSync(bool enable) override { }
		virtual void SetRequiredExtensions(std::vector<const char*> extensions) override { }

		static OpenGLContext* Get() { return s_ContextInstance; }
	protected:
		virtual CommandBuffer* GetCommandBuffer();
	private:
		bool CheckExtensionsSupport();
		void CreateCommandBuffers();
		void DestroyCommandBuffers();
	private:
		GLFWwindow*					m_WindowHandle;
		static OpenGLContext*		s_ContextInstance;
		std::vector<const char*>	m_Extensions =
		{

		};
		std::vector<OpenGLCommandBuffer*>	m_CommandBuffers;
		uint32_t							m_CommandBufferIndex = 0;

		friend class OpenGLRendererAPI;
	};
}
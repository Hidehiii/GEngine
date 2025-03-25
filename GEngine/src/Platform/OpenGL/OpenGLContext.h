#pragma once

#include "GEngine/Graphics/GraphicsContext.h"
#include "OpenGLCommandBuffer.h"
#include <GLFW/glfw3.h>

namespace GEngine
{
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
		virtual Ref<OpenGLCommandBuffer> GetCommandBuffer(CommandBufferType type);
	private:
		bool CheckExtensionsSupport();
		void CreateCommandBuffers();
	private:
		GLFWwindow*					m_WindowHandle;
		static OpenGLContext*		s_ContextInstance;
		std::vector<const char*>	m_Extensions =
		{

		};
		std::vector<Ref<OpenGLCommandBuffer>>	m_GraphicsCommandBuffers;
		uint32_t								m_GraphicsCommandBufferIndex = 0;
		std::vector<Ref<OpenGLCommandBuffer>>	m_ComputeCommandBuffers;
		uint32_t								m_ComputeCommandBufferIndex = 0;

		friend class OpenGLRendererAPI;
	};
}
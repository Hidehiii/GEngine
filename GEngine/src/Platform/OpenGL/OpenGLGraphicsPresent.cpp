#include "GEpch.h"
#include "OpenGLGraphicsPresent.h"
#include "OpenGLCommandBuffer.h"
#include "GEngine/Graphics/Graphics.h"
#include <glad/glad.h>

namespace GEngine
{
	OpenGLGraphicsPresent::OpenGLGraphicsPresent()
	{
		for (int i = 0; i < Graphics::GetFramesInFlight(); i++)
		{
			s_CommandBuffers.push_back(OpenGLCommandBuffer::Create(CommandBufferType::Graphics));
		}
	}
	void OpenGLGraphicsPresent::Begin()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glViewport(0, 0, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	}
	void OpenGLGraphicsPresent::End()
	{
		glDisable(GL_FRAMEBUFFER_SRGB);
	}
}
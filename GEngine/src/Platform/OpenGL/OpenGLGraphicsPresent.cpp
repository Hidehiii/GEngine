#include "GEpch.h"
#include "OpenGLGraphicsPresent.h"
#include <glad/glad.h>

namespace GEngine
{
	void OpenGLGraphicsPresent::Begin()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void OpenGLGraphicsPresent::End()
	{
		glDisable(GL_FRAMEBUFFER_SRGB);
	}
}
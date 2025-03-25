#pragma once
#include "GEngine/Graphics/GraphicsPresent.h"

namespace GEngine
{
	class GENGINE_API OpenGLGraphicsPresent : public GraphicsPresent
	{
	public:
		OpenGLGraphicsPresent();
		virtual void Begin() override;
		virtual void End() override;
	};
}



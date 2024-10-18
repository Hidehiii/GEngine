#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Texture.h"

namespace GEngine
{
	class GENGINE_API GUIUtils
	{
	public:
		static void* GetTextureID(const Ref<Texture2D>& texture);
	};
}
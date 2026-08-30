#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Math/Math.h"
namespace GEngine
{
	class GENGINE_API GUIUtils
	{
	public:
		static void* GetTextureID(const Ref<Texture2D>& texture);
		static Vector2 GetUV0();
		static Vector2 GetUV1();
	};
}
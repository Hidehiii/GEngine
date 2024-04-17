#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Texture.h"
#include "GEngine/Renderer/SubTexture2D.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	class GENGINE_API ImageRenderer : public Component
	{
	public:
		ImageRenderer(Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f })
			: m_Color(color) {}
		ImageRenderer(float r, float g, float b, float a)
			: m_Color({ r, g, b, a }) {}
		ImageRenderer(float value)
			: m_Color({ value, value, value, value }) {}

		ImageRenderer(const ImageRenderer&) = default;

		operator bool() { return this == NULL; }
	public:
		Vector4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> m_Texture;
		Vector2 m_Tiling = { 1.0f, 1.0f };
	};
}



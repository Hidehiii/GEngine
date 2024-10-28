#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Texture.h"
#include "GEngine/Renderer/SubTexture2D.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	// Contains the image renderer of a game object
	// Renders the image of the game object
	// Can be used to render a color or a texture
	// The shpae of the image is a quad
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

		void OnRender();

		operator bool() { return this == NULL; }
	public:
		// If a texture is used, the color will be multiplied by the texture color
		Vector4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		// The texture to render
		Ref<Texture2D> m_Texture;
		// The tiling of the texture
		Vector2 m_Tiling = { 1.0f, 1.0f };
	};
}



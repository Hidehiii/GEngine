#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/Texture.h"
namespace GEngine
{
	// Contains the circle renderer of a game object
	// Renders the circle of the game object
	// Can be used to render a color or a texture
	// The shape of the circle is a quad
	// The circle is rendered using a shader
	class GENGINE_API CircleRenderer : public Component
	{
	public:
		CircleRenderer() = default;
		CircleRenderer(const CircleRenderer&) = default;

		void OnRender();

	public:
		// If a texture is used, the color will be multiplied by the texture color
		Vector4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		// The texture to render
		Ref<Texture2D> m_Texture = nullptr;
		// The radius of the circle
		float m_Radius = 0.5f;
		// The fade of the circle
		float m_Fade = 0.0f;
		// The thickness of the circle
		float m_Thickness = 1.0f;
	};
}



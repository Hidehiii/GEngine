#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/Texture.h"
namespace GEngine
{
	class GENGINE_API CircleRenderer : public Component
	{
	public:
		CircleRenderer() = default;
		CircleRenderer(const CircleRenderer&) = default;

	public:
		Vector4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> m_Texture = nullptr;
		float m_Radius = 0.5f;
		float m_Fade = 0.0f;
		float m_Thickness = 1.0f;
	};
}



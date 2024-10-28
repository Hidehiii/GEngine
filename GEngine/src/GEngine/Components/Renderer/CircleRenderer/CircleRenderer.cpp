#include "GEpch.h"
#include "CircleRenderer.h"
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/Renderer2D.h"

namespace GEngine
{
	void CircleRenderer::OnRender()
	{
		Renderer2D::RenderCircle(m_GameObject.GetComponent<Transform>(), *this);
	}
}
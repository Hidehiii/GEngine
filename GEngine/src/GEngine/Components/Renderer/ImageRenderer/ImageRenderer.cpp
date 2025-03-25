#include "GEpch.h"
#include "ImageRenderer.h"
#include "GEngine/Graphics/Renderer.h"
#include "GEngine/Graphics/Renderer2D.h"

namespace GEngine
{
	void ImageRenderer::OnRender()
	{
		Renderer2D::RenderImage(m_GameObject.GetComponent<Transform>(), *this);
	}
}
#include "GEpch.h"
#include "ImageRenderer.h"
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/Renderer2D.h"

namespace GEngine
{
	void ImageRenderer::OnRender()
	{
		Renderer2D::RenderImage(m_GameObject.GetComponent<Transform>(), *this);
	}
}
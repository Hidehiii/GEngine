#include "GpuDriven.h"

namespace GEngine
{
	GpuDriven::GpuDriven()
	{
	}
	void GpuDriven::OnAttach()
	{

		m_EditorCamera = Editor::EditorCamera(30.0f, 1.0f, 0.01f, 10000.0f);
	}
	void GpuDriven::OnPresent()
	{
	}
	void GpuDriven::OnRender()
	{
	}
	void GpuDriven::OnUpdate()
	{
	}
	void GpuDriven::OnImGuiRender()
	{
	}
	void GpuDriven::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
	}
}
#include "GEpch.h"
#include "DirectionalLight.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
    void DirectionalLight::OnRender()
    {
		Vector3 main_dir = { -1.0f, -1.0f, 0.0f };
		Vector3 main_color = { 1.0f, 1.0f, 1.0f };
		if (m_GameObject)
		{
			main_dir = Math::Rotate(m_GameObject.GetComponent<Transform>().m_Rotation, { 0.0f, 0.0f, -1.0f });
			main_color = m_Color;
		}
    }
}
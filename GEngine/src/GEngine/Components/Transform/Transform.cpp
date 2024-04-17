#include "GEpch.h"
#include "Transform.h"

namespace GEngine
{
	void Transform::RecalculateModelMatrix()
	{
		m_ModelMatrix = Matrix4x4(1.0f);
		m_ModelMatrix *= Math::Translate(Matrix4x4(1.0f), m_Position);
		m_ModelMatrix *= Math::ToMatrix4x4(m_Rotation);
		m_ModelMatrix *= Math::Scale(Matrix4x4(1.0f), m_Scale);
	}
}
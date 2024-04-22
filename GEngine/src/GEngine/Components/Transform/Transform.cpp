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
	Vector3 Transform::Forward()
	{
		return m_Rotation * Vector3(0.0f, 0.0f, -1.0f);
	}
	Vector3 Transform::Right()
	{
		return m_Rotation * Vector3(1.0f, 0.0f, 0.0f);
	}
	Vector3 Transform::Up()
	{
		return m_Rotation * Vector3(0.0f, 1.0f, 0.0f);
	}
}
#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"


namespace GEngine
{
	// Contains the transform of a game object
	class GENGINE_API Transform : public Component
	{
	public:
		Transform(Vector3 position = { 0.0f, 0.0f, 0.0f }, Vector3 angles = { 0.0f, 0.0f, 0.0f }, Vector3 scale = { 1.0f, 1.0f, 1.0f })
			: m_Position(position), m_Scale(scale), m_ModelMatrix(1.0f), m_Rotation(Math::Radians(angles))
		{
			RecalculateModelMatrix();
		}

		Transform(const Transform&) = default;


		void RecalculateModelMatrix();

		Matrix4x4 GetModelMatrix() { RecalculateModelMatrix(); return m_ModelMatrix; }
		Matrix4x4 GetTransformMatrix() { RecalculateModelMatrix(); return m_ModelMatrix; }

		Matrix4x4 GetTransformMatrix() const;
		Matrix4x4 GetRotationMatrix() const;
		Matrix4x4 GetScaleMatrix() const;

		Vector3 GetPosition() const { return m_Position; }
		Quaternion GetRotation() const { return m_Rotation; }
		Vector3 GetScale() const { return m_Scale; }

		void SetPosition(const Vector3& position) { m_Position = position; RecalculateModelMatrix(); }
		void SetRotation(const Vector3& angles) { m_Rotation = Quaternion(Math::Radians(angles)); RecalculateModelMatrix(); }
		void SetRotation(const Quaternion& rotation) { m_Rotation = rotation; RecalculateModelMatrix(); }
		void SetScale(const Vector3& scale) { m_Scale = scale; RecalculateModelMatrix(); }

		void Translate(const Vector3& translation) { m_Position += translation; RecalculateModelMatrix(); }
		void Rotate(const Vector3& angles) { m_Rotation = Quaternion(Math::EulerAngles(m_Rotation) + Math::Radians(angles)); RecalculateModelMatrix(); }
		void Rotate(const Quaternion& quat) { m_Rotation += quat; m_Rotation = Math::Normalized(m_Rotation); RecalculateModelMatrix(); }
		void Scale(const Vector3& scale) { m_Scale *= scale; RecalculateModelMatrix(); }

		void SetModelMatrix(const Matrix4x4& modelMatrix) { m_ModelMatrix = modelMatrix; }

		Vector3 Forward();
		Vector3 Right();
		Vector3 Up();

		Vector3 GetEulerAngle() { return Math::EulerAngles(m_Rotation); }
		void SetEulerAngleInRadians(const Vector3 radians) { m_Rotation = Quaternion(radians);  RecalculateModelMatrix(); }
		void SetEulerAngle(const Vector3 angles) { m_Rotation = Quaternion(Math::Radians(angles)); RecalculateModelMatrix(); }
	public:
		Vector3 m_Position = { 0.0f, 0.0f, 0.0f};
		Quaternion m_Rotation = Vector3{ 0.0f, 0.0f, 0.0f};
		Vector3 m_Scale = { 1.0f, 1.0f, 1.0f};
	private:
		Matrix4x4 m_ModelMatrix = Matrix4x4(1.0f);
	};
}



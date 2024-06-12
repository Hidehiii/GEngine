#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Vector2.h"
#include "GEngine/Math/Vector3.h"
#include "GEngine/Math/Vector4.h"
#include "GEngine/Math/Matrix4x4.h"
#include "GEngine/Math/Quaternion.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
namespace GEngine
{
	class GENGINE_API Math
	{
	public:
		static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float nearValue, float farValue);
		static Matrix4x4 Perspective(float fov, float aspectRatio, float neaValuer, float farValue);
		static Matrix4x4 Translate(const Vector3& translation);
		static Matrix4x4 Translate(const Matrix4x4& transform, const Vector3& translation);
		static Matrix4x4 Rotate(float angle, const Vector3& axis);
		static Vector3 Rotate(Quaternion quat, Vector3 v);
		static Matrix4x4 Rotate(const Matrix4x4& transform, float angle, const Vector3& axis);
		static Matrix4x4 Scale(const Vector3& scale);
		static Matrix4x4 Scale(const Matrix4x4& transform, const Vector3& scale);
		static float Radians(const float angle);
		static Vector2 Radians(const Vector2 angle);
		static Vector3 Radians(const Vector3 angle);
		static Vector4 Radians(const Vector4 angle);
		static Matrix4x4 Inverse(const Matrix4x4& matrix);
		static float Degrees(const float angle);
		static Vector2 Degrees(const Vector2 angle);
		static Vector3 Degrees(const Vector3 angle);
		static Vector4 Degrees(const Vector4 angle);
		static Matrix4x4 ToMatrix4x4(Quaternion quat);
		static Matrix4x4 ToMatrix4x4(Vector3 rotation);
		static bool DecomposeTransformMatrix(const Matrix4x4 transformMatrix, Vector3& outPosition, Quaternion& outRotation, Vector3& outScale);
		static bool DecomposeTransformMatrix(const Matrix4x4 transformMatrix, Vector3& outPosition, Vector3& outRotation, Vector3& outScale);
		static Vector3 EulerAngles(Quaternion quat);
		static float Length(const Vector2& v);
		static float Length(const Vector3& v);
		static float Length(const Vector4& v);
		static float Max(const Vector2& v);
		static float Max(const Vector3& v);
		static float Max(const Vector4& v);
		static float Dot(const Vector2& v1, const Vector2& v2);
		static float Dot(const Vector3& v1, const Vector3& v2);
		static float Dot(const Vector4& v1, const Vector4& v2);
		static Vector3 Clamp(const Vector3& value, const Vector3& min, const Vector3& max);
		static float Clamp(float value, float min, float max);
		static Vector3 Reflect(const Vector3& v, const Vector3& normal);
		static float Sqrt(float value);
		static float Tan(float angle);
		static Matrix4x4 IdentityMatrix();
		static float Sin(float angle);
		static float Cos(float angle);

		template<typename T>
		static T Max(std::initializer_list<T> list)
		{
			T max = *list.begin();
			for (auto& value : list)
			{
				if (value > max)
					max = value;
			}
			return max;
		}
		template<typename T>
		static T Min(std::initializer_list<T> list)
		{
			T min = *list.begin();
			for (auto& value : list)
			{
				if (value < min)
					min = value;
			}
			return min;
		}

	public:
		static float PI;
	};
}


#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)

template<typename OStream>
inline OStream& operator<<(OStream& os, const GEngine::Vector2 vector)
{
	return os << glm::to_string(vector.value);
}

template<typename OStream>
inline OStream& operator<<(OStream& os, const GEngine::Vector3 vector)
{
	return os << glm::to_string(vector.value);
}

template<typename OStream>
inline OStream& operator<<(OStream& os, const GEngine::Vector4 vector)
{
	return os << glm::to_string(vector.value);
}

template<typename OStream>
inline OStream& operator<<(OStream& os, const GEngine::Matrix4x4& matrix)
{
	return os << glm::to_string(matrix.value);
}

template<typename OStream>
inline OStream& operator<<(OStream& os, GEngine::Quaternion quaternion)
{
	return os << glm::to_string(quaternion.value);
}
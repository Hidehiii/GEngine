#pragma once

#include "GEngine/Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GEngine
{
	class Vector2;
	class Vector3;

	class GENGINE_API Vector4
	{
	public:
		Vector4(float v = 0);
		Vector4(glm::vec4 v);
		Vector4(Vector2 v, float z = 0, float w = 0);
		Vector4(Vector3 v, float w = 0);
		Vector4(const Vector4&) = default;
		Vector4(float x, float y, float z, float w);
		~Vector4();

		Vector4& Add(const Vector4& other);
		Vector4& Subtract(const Vector4& other);
		Vector4& Multiply(const Vector4& other);
		Vector4& Divide(const Vector4& other);

		Vector4 operator+(const Vector4& other);
		Vector4 operator-(const Vector4& other);
		Vector4 operator*(const Vector4& other);
		Vector4 operator/(const Vector4& other);

		Vector4 operator+(const float& other);
		Vector4 operator-(const float& other);
		Vector4 operator*(const float& other);
		Vector4 operator/(const float& other);

		Vector4& operator+=(const Vector4& other);
		Vector4& operator-=(const Vector4& other);
		Vector4& operator*=(const Vector4& other);
		Vector4& operator/=(const Vector4& other);

		Vector4 Normalized();

		bool operator==(const Vector4& other);
		bool operator!=(const Vector4& other);

		operator glm::vec4&() { return value; }
		operator const glm::vec4&() const { return value; }

		float* ValuePtr() const { return (float*)glm::value_ptr(value); }

		glm::vec4 value;
	};
}



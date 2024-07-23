#pragma once

#include "GEngine/Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GEngine
{
	class GENGINE_API Vector2
	{
	public:
		Vector2(float v = 0);
		Vector2(glm::vec2 vector);
		Vector2(const Vector2&) = default;
		Vector2(float x, float y);
		~Vector2();

		Vector2& Add(const Vector2& other);
		Vector2& Subtract(const Vector2& other);
		Vector2& Multiply(const Vector2& other);
		Vector2& Divide(const Vector2& other);

		Vector2 operator+(const Vector2& other);
		Vector2 operator-(const Vector2& other);
		Vector2 operator*(const Vector2& other);
		Vector2 operator/(const Vector2& other);

		friend Vector2 operator+(const Vector2& left, Vector2 right);
		friend Vector2 operator-(const Vector2& left, Vector2 right);
		friend Vector2 operator*(const Vector2& left, Vector2 right);
		friend Vector2 operator/(const Vector2& left, Vector2 right);

		Vector2 operator+(const float& other);
		Vector2 operator-(const float& other);
		Vector2 operator*(const float& other);
		Vector2 operator/(const float& other);

		Vector2& operator+=(const Vector2& other);
		Vector2& operator-=(const Vector2& other);
		Vector2& operator*=(const Vector2& other);
		Vector2& operator/=(const Vector2& other);

		Vector2 Normalized();

		bool operator==(const Vector2& other);
		bool operator!=(const Vector2& other);
		Vector2 operator-();

		operator glm::vec2&() { return value; }
		operator const glm::vec2&() const { return value; }

		float* ValuePtr() const { return (float*)glm::value_ptr(value); }

		glm::vec2 value;
	};
}



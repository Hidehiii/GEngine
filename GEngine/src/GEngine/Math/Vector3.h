#pragma once

#include "GEngine/Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GEngine/Math/Vector4.h"

namespace GEngine
{
	class Quaternion;
	class Vector2;

	class GENGINE_API Vector3
	{
	public:
		Vector3(float v = 0);
		Vector3(glm::vec3 vector);
		Vector3(Vector2 v, float z);
		Vector3(const Vector3&) = default;
		Vector3(float x, float y, float z);
		Vector3(Vector4 c);

		~Vector3();

		Vector3& Add(const Vector3& other);
		Vector3& Subtract(const Vector3& other);
		Vector3& Multiply(const Vector3& other);
		Vector3& Divide(const Vector3& other);

		Vector3 operator+(const Vector3& other);
		Vector3 operator-(const Vector3& other);
		Vector3 operator*(const Vector3& other);
		Vector3 operator/(const Vector3& other);

		friend Vector3 operator+(const Vector3& left, Vector3 right);
		friend Vector3 operator-(const Vector3& left, Vector3 right);
		friend Vector3 operator*(const Vector3& left, Vector3 right);
		friend Vector3 operator/(const Vector3& left, Vector3 right);

		GENGINE_API friend Vector3 operator+(const float& left, const Vector3 right);
		GENGINE_API friend Vector3 operator-(const float& left, const Vector3 right);
		GENGINE_API friend Vector3 operator*(const float& left, const Vector3 right);
		GENGINE_API friend Vector3 operator/(const float& left, const Vector3 right);

		Vector3 operator+(const float& other);
		Vector3 operator-(const float& other);
		Vector3 operator*(const float& other);
		Vector3 operator/(const float& other);

		Vector3& operator+=(const Vector3& other);
		Vector3& operator-=(const Vector3& other);
		Vector3& operator*=(const Vector3& other);
		Vector3& operator/=(const Vector3& other);

		Vector3 Normalized();

		bool operator==(const Vector3& other);
		bool operator!=(const Vector3& other);
		Vector3 operator-();

		operator glm::vec3&() { return value; }
		operator const glm::vec3&() const { return value; }

		float* ValuePtr() const { return (float*)glm::value_ptr(value); }

		glm::vec3 value;
	};

	
}




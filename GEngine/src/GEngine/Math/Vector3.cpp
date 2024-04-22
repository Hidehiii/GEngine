#include "GEpch.h"
#include "Vector3.h"
#include "Vector2.h"

namespace GEngine
{

	Vector3::Vector3(float v)
	{
		value = glm::vec3(v);
	}
	Vector3::Vector3(glm::vec3 vector)
	{
		value = vector;
	}
	Vector3::Vector3(Vector2 v, float z)
	{
		value = glm::vec3(v.value, z);
	}
	Vector3::Vector3(float x, float y, float z)
	{
		value = glm::vec3(x, y, z);
	}
	Vector3::Vector3(Vector4 c)
	{
		value = glm::vec3(c.value);
	}
	Vector3::~Vector3()
	{
	}
	Vector3& Vector3::Add(const Vector3& other)
	{
		this->value += other.value;
		return *this;
	}
	Vector3& Vector3::Subtract(const Vector3& other)
	{
		this->value -= other.value;
		return *this;
	}
	Vector3& Vector3::Multiply(const Vector3& other)
	{
		this->value *= other.value;
		return *this;
	}
	Vector3& Vector3::Divide(const Vector3& other)
	{
		this->value /= other.value;
		return *this;
	}
	Vector3 Vector3::operator+(const Vector3& other)
	{
		return this->value + other.value;
	}
	Vector3 Vector3::operator-(const Vector3& other)
	{
		return this->value - other.value;
	}
	Vector3 Vector3::operator*(const Vector3& other)
	{
		return this->value * other.value;
	}
	Vector3 Vector3::operator/(const Vector3& other)
	{
		return this->value / other.value;	
	}
	Vector3 Vector3::operator+(const float& other)
	{
		return this->value + other;
	}
	Vector3 Vector3::operator-(const float& other)
	{
		return this->value - other;
	}
	Vector3 Vector3::operator*(const float& other)
	{
		return this->value * other;
	}
	Vector3 Vector3::operator/(const float& other)
	{
		return this->value / other;
	}
	Vector3& Vector3::operator+=(const Vector3& other)
	{
		value += other.value;
		return *this;
	}
	Vector3& Vector3::operator-=(const Vector3& other)
	{
		value -= other.value;
		return *this;
	}
	Vector3& Vector3::operator*=(const Vector3& other)
	{
		value *= other.value;
		return *this;
	}
	Vector3& Vector3::operator/=(const Vector3& other)
	{
		value /= other.value;
		return *this;
	}
	Vector3 Vector3::Normalized()
	{
		return glm::normalize(value);
	}
	bool Vector3::operator==(const Vector3& other)
	{
		return this->value == other.value;
	}
	bool Vector3::operator!=(const Vector3& other)
	{
		return this->value != other.value;
	}
	Vector3 Vector3::operator-()
	{
		return Vector3(-value);
	}

	Vector3 operator+(const Vector3& left, Vector3 right)
	{
		return left.value + right.value;
	}

	Vector3 operator-(const Vector3& left, Vector3 right)
	{
		return left.value - right.value;
	}

	Vector3 operator*(const Vector3& left, Vector3 right)
	{
		return left.value * right.value;
	}

	Vector3 operator/(const Vector3& left, Vector3 right)
	{
		return left.value / right.value;
	}

	Vector3 operator+(const float& left, const Vector3 right)
	{
		return left + right.value;
	}

	Vector3 operator-(const float& left, const Vector3 right)
	{
		return left - right.value;
	}

	Vector3 operator*(const float& left, const Vector3 right)
	{
		return left * right.value;
	}

	Vector3 operator/(const float& left, const Vector3 right)
	{
		return left / right.value;
	}

}

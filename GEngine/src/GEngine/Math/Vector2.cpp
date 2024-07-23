#include "GEpch.h"
#include "Vector2.h"

namespace GEngine
{
	Vector2::Vector2(float v)
	{
		value = glm::vec2(v);
	}
	Vector2::Vector2(glm::vec2 vector)
	{
		value = vector;
	}
	Vector2::Vector2(float x, float y)
	{
		value = glm::vec2(x, y);
	}
	Vector2::~Vector2()
	{
	}
	Vector2& Vector2::Add(const Vector2& other)
	{
		value += other.value;
		return *this;
	}
	Vector2& Vector2::Subtract(const Vector2& other)
	{
		value -= other.value;
		return *this;
	}
	Vector2& Vector2::Multiply(const Vector2& other)
	{
		value *= other.value;
		return *this;
	}
	Vector2& Vector2::Divide(const Vector2& other)
	{
		value /= other.value;
		return *this;
	}
	Vector2 Vector2::operator+(const Vector2& other)
	{
		return this->value + other.value;
	}
	Vector2 Vector2::operator-(const Vector2& other)
	{
		return this->value - other.value;
	}
	Vector2 Vector2::operator*(const Vector2& other)
	{
		return this->value * other.value;
	}
	Vector2 Vector2::operator/(const Vector2& other)
	{
		return this->value / other.value;
	}
	Vector2 Vector2::operator+(const float& other)
	{
		return this->value + other;
	}
	Vector2 Vector2::operator-(const float& other)
	{
		return this->value - other;
	}
	Vector2 Vector2::operator*(const float& other)
	{
		return this->value * other;
	}
	Vector2 Vector2::operator/(const float& other)
	{
		return this->value / other;
	}
	Vector2& Vector2::operator+=(const Vector2& other)
	{
		this->value += other.value;
		return *this;
	}
	Vector2& Vector2::operator-=(const Vector2& other)
	{
		this->value -= other.value;
		return *this;
	}
	Vector2& Vector2::operator*=(const Vector2& other)
	{
		this->value *= other.value;
		return *this;
	}
	Vector2& Vector2::operator/=(const Vector2& other)
	{
		this->value /= other.value;
		return *this;
	}
	Vector2 Vector2::Normalized()
	{
		return glm::normalize(value);
	}
	bool Vector2::operator==(const Vector2& other)
	{
		return this->value == other.value;
	}
	bool Vector2::operator!=(const Vector2& other)
	{
		return this->value != other.value;
	}
	Vector2 Vector2::operator-()
	{
		return Vector2(-value);
	}
	Vector2 operator+(const Vector2& left, Vector2 right)
	{
		return Vector2(left.value + right.value);
	}
	Vector2 operator-(const Vector2& left, Vector2 right)
	{
		return Vector2(left.value - right.value);
	}
	Vector2 operator*(const Vector2& left, Vector2 right)
	{
		return Vector2(left.value * right.value);
	}
	Vector2 operator/(const Vector2& left, Vector2 right)
	{
		return Vector2(left.value / right.value);
	}
}

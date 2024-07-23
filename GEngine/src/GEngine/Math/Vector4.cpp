#include "GEpch.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"

namespace GEngine
{
    GEngine::Vector4::Vector4(float v)
    {
        value = glm::vec4(v);
    }

    Vector4::Vector4(glm::vec4 v)
    {
        value = v;
    }

    Vector4::Vector4(Vector2 v, float z, float w)
    {
        value = glm::vec4(v.value, z, w);
    }

    Vector4::Vector4(Vector3 v, float w)
    {
		value = glm::vec4(v.value, w);
    }

    GEngine::Vector4::Vector4(float x, float y, float z, float w)
    {
        value = glm::vec4(x, y, z, w);
    }

    GEngine::Vector4::~Vector4()
    {
    }

    Vector4& GEngine::Vector4::Add(const Vector4& other)
    {
        value += other.value;
        return *this;
    }

    Vector4& GEngine::Vector4::Subtract(const Vector4& other)
    {
        value -= other.value;
        return *this;
    }

    Vector4& GEngine::Vector4::Multiply(const Vector4& other)
    {
        value *= other.value;
        return *this;
    }

    Vector4& GEngine::Vector4::Divide(const Vector4& other)
    {
        value /= other.value;
        return *this;
    }

    Vector4 Vector4::operator+(const Vector4& other)
    {
        return this->value + other.value;
    }

    Vector4 Vector4::operator-(const Vector4& other)
    {
        return this->value - other.value;
    }

    Vector4 Vector4::operator*(const Vector4& other)
    {
        return this->value * other.value;
    }

    Vector4 Vector4::operator/(const Vector4& other)
    {
        return this->value / other.value;
    }

    Vector4 Vector4::operator+(const float& other)
    {
        return this->value + other;
    }

    Vector4 Vector4::operator-(const float& other)
    {
        return this->value - other;
    }

    Vector4 Vector4::operator*(const float& other)
    {
        return this->value * other;
    }

    Vector4 Vector4::operator/(const float& other)
    {
        return this->value / other;
    }

    Vector4& GEngine::Vector4::operator+=(const Vector4& other)
    {
        value += other.value;
        return *this;
    }

    Vector4& GEngine::Vector4::operator-=(const Vector4& other)
    {
        value -= other.value;
        return *this;
    }

    Vector4& GEngine::Vector4::operator*=(const Vector4& other)
    {
        value *= other.value;
        return *this;
    }

    Vector4& GEngine::Vector4::operator/=(const Vector4& other)
    {
        value /= other.value;
        return *this;
    }

    Vector4 Vector4::Normalized()
    {
        return glm::normalize(value);
    }

    bool GEngine::Vector4::operator==(const Vector4& other)
    {
        return this->value == other.value;
    }

    bool GEngine::Vector4::operator!=(const Vector4& other)
    {
        return this->value != other.value;
    }

}

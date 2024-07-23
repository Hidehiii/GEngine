#include "GEpch.h"
#include "Matrix4x4.h"

namespace GEngine
{
    Matrix4x4::Matrix4x4()
    {
		value = glm::mat4(1.0f);
    }

    Matrix4x4::Matrix4x4(glm::mat4 matrix)
    {
        value = matrix;
    }

    Matrix4x4::Matrix4x4(float diagonal)
    {
        value = glm::mat4(diagonal);
    }

    Matrix4x4::Matrix4x4(Vector4 a, Vector4 b, Vector4 c, Vector4 d)
    {
        value = glm::mat4(a.value, b.value, c.value, d.value);
    }

    Matrix4x4::~Matrix4x4()
    {
    }

    Matrix4x4& Matrix4x4::Multiply(const Matrix4x4& other)
    {
        this->value *= other.value;
        return *this;
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other)
    {
        return this->value * other.value;
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other)
    {
        value *= other.value;
        return *this;
    }
    Vector4 Matrix4x4::operator[](int i)
    {
        return Vector4(value[i]);
    }
    Vector4 Matrix4x4::operator*(const Vector4& vector)
    {
        return value * vector.value;
    }
    Matrix4x4 operator*(const Matrix4x4& left, Matrix4x4 right)
    {
        return left.value * right.value;
    }
}


#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Vector4.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GEngine
{
	class GENGINE_API Matrix4x4
	{
	public:
		Matrix4x4();
		Matrix4x4(glm::mat4 matrix);
		Matrix4x4(float diagonal);
		Matrix4x4(Vector4 a, Vector4 b, Vector4 c, Vector4 d);
		~Matrix4x4();

		Matrix4x4& Multiply(const Matrix4x4& other);
		Matrix4x4 operator*(const Matrix4x4& other);
		Matrix4x4& operator*=(const Matrix4x4& other);

		Vector4 operator[](int i);
		Vector4 operator*(const Vector4& vector);

		friend Matrix4x4 operator*(const Matrix4x4& left,Matrix4x4 right);

		operator glm::mat4&() { return value; }
		operator const glm::mat4&() const { return value; }

		float* ValuePtr() const { return (float*)glm::value_ptr(value); }

		glm::mat4 value;
	};
}



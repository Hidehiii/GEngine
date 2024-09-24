#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Vector3.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>


namespace GEngine
{
	class GENGINE_API Quaternion
	{
	public:
		Quaternion();
		Quaternion(Vector3 v);
		Quaternion(glm::quat q) { value = q;}
		Quaternion(float w, float x, float y, float z);
		Quaternion(float x, float y, float z);

		Quaternion Normalized();

		Quaternion operator+(const Quaternion& q);

		Quaternion operator+=(const Quaternion& q);

		Vector3 operator*(const Vector3& v);

	public:
		glm::quat value = glm::vec3(0.0f, 0.0f, 0.0f);
	};
}



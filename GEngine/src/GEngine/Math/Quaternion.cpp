#include "GEpch.h"
#include "Quaternion.h"


namespace GEngine
{
    Quaternion::Quaternion()
    {
    }
    Quaternion::Quaternion(Vector3 v)
    {
        value = glm::quat(v.value);
    }
    Quaternion::Quaternion(float w, float x, float y, float z)
    {
        value = glm::quat(w, x, y, z);
    }
    Quaternion::Quaternion(float x, float y, float z)
    {
        value = glm::quat(glm::vec3(x, y, z));
    }
    Quaternion Quaternion::Normalized()
    {
        return glm::normalize(value);
    }
    Quaternion Quaternion::operator+(const Quaternion& q)
    {
        return value + q.value;
    }
    Quaternion Quaternion::operator+=(const Quaternion& q)
    {
        return this->value += q.value;
    }
    Vector3 Quaternion::operator*(const Vector3& v)
    {
        return value * v.value;
    }
}
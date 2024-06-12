#include "GEpch.h"
#include "Math.h"


namespace GEngine
{
    float Math::PI = glm::pi<float>();

    Matrix4x4 Math::Orthographic(float left, float right, float bottom, float top, float nearValue, float farValue)
    {
        return glm::ortho(left, right, bottom, top, nearValue, farValue);
    }
    Matrix4x4 Math::Perspective(float fov, float aspectRatio, float nearValue, float farValue)
    {
        return glm::perspective(glm::radians(fov), aspectRatio, nearValue, farValue);
    }
    Matrix4x4 Math::Translate(const Vector3& translation)
    {
        return glm::translate(glm::mat4(1.0f), translation.value);
    }
    Matrix4x4 Math::Translate(const Matrix4x4& transform, const Vector3& translation)
    {
        return glm::translate(transform.value, translation.value);
    }
    Matrix4x4 Math::Rotate(float angle, const Vector3& axis)
    {
        return glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis.value);
    }
    Vector3 Math::Rotate(Quaternion quat, Vector3 v)
    {
        return glm::rotate(quat.value, v.value);
    }
    Matrix4x4 Math::Rotate(const Matrix4x4& transform, float angle, const Vector3& axis)
    {
        return glm::rotate(transform.value, glm::radians(angle), axis.value);
    }
    Matrix4x4 Math::Scale(const Vector3& scale)
    {
		return glm::scale(glm::mat4(1.0f), scale.value);
	}
    Matrix4x4 Math::Scale(const Matrix4x4& transform, const Vector3& scale)
    {
        return glm::scale(transform.value, scale.value);
    }
    float Math::Radians(const float angle)
    {
        return glm::radians(angle);
    }
    Vector2 Math::Radians(const Vector2 angle)
    {
        return glm::radians(angle.value);
    }
    Vector3 Math::Radians(const Vector3 angle)
    {
        return glm::radians(angle.value);
    }
    Vector4 Math::Radians(const Vector4 angle)
    {
        return glm::radians(angle.value);
    }
    Matrix4x4 Math::Inverse(const Matrix4x4& matrix)
    {
        return glm::inverse(matrix.value);
    }
    float Math::Degrees(const float angle)
    {
        return glm::degrees(angle);
    }
    Vector2 Math::Degrees(const Vector2 angle)
    {
        return glm::degrees(angle.value);
    }
    Vector3 Math::Degrees(const Vector3 angle)
    {
        return glm::degrees(angle.value);
    }
    Vector4 Math::Degrees(const Vector4 angle)
    {
        return glm::degrees(angle.value);
    }
    Matrix4x4 Math::ToMatrix4x4(Quaternion quat)
    {
        return Matrix4x4(glm::toMat4(quat.value));
    }
    Matrix4x4 Math::ToMatrix4x4(Vector3 rotation)
    {
        return Matrix4x4(glm::toMat4(Quaternion(rotation).value));
    }
    bool Math::DecomposeTransformMatrix(const Matrix4x4 transformMatrix, Vector3& outPosition, Quaternion& outRotation, Vector3& outScale)
    {
        // From glm::decompose in matrix_decompose.inl
        using namespace glm;
        using T = float;

        mat4 LocalMatrix(transformMatrix.value);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
			return false;


		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		outPosition = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		outScale.value.x = length(Row[0]);// v3Length(Row[0]);

		Row[0] = detail::scale(Row[0], static_cast<T>(1));

		// Now, compute Y scale and normalize 2nd row.
		outScale.value.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));


		// Next, get Z scale and normalize 3rd row.
		outScale.value.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));


		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.

		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
                outScale.value[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}


        //outRotation.value.y = asin(-Row[0][2]);
        //if (cos(outRotation.value.y) != 0)
        //{
        //    outRotation.value.x = atan2(Row[1][2], Row[2][2]);
        //    outRotation.value.z = atan2(Row[0][1], Row[0][0]);
        //}
        //else
        //{
        //    outRotation.value.x = atan2(-Row[2][0], Row[1][1]);
        //    outRotation.value.z = 0;
        //}

        int i, j, k = 0;
        T root, trace = Row[0].x + Row[1].y + Row[2].z;
        if (trace > static_cast<T>(0))
        {
            root = sqrt(trace + static_cast<T>(1.0));
            outRotation.value.w = static_cast<T>(0.5) * root;
            root = static_cast<T>(0.5) / root;
            outRotation.value.x = root * (Row[1].z - Row[2].y);
            outRotation.value.y = root * (Row[2].x - Row[0].z);
            outRotation.value.z = root * (Row[0].y - Row[1].x);
        } // End if > 0
        else
        {
            static int Next[3] = { 1, 2, 0 };
            i = 0;
            if (Row[1].y > Row[0].x) i = 1;
            if (Row[2].z > Row[i][i]) i = 2;
            j = Next[i];
            k = Next[j];


#           ifdef GLM_FORCE_QUAT_DATA_WXYZ
            int off = 1;
#           else
            int off = 0;
#           endif

            root = sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

            outRotation.value[i + off] = static_cast<T>(0.5) * root;
            root = static_cast<T>(0.5) / root;
            outRotation.value[j + off] = root * (Row[i][j] + Row[j][i]);
            outRotation.value[k + off] = root * (Row[i][k] + Row[k][i]);
            outRotation.value.w = root * (Row[j][k] - Row[k][j]);
        } // End if <= 0

        return true;
    }
    bool Math::DecomposeTransformMatrix(const Matrix4x4 transformMatrix, Vector3& outPosition, Vector3& outRotation, Vector3& outScale)
    {
        // From glm::decompose in matrix_decompose.inl
        using namespace glm;
        using T = float;

        mat4 LocalMatrix(transformMatrix.value);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
            return false;


        // First, isolate perspective.  This is the messiest.
        if (
            epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        outPosition = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        outScale.value.x = length(Row[0]);// v3Length(Row[0]);

        Row[0] = detail::scale(Row[0], static_cast<T>(1));

        // Now, compute Y scale and normalize 2nd row.
        outScale.value.y = length(Row[1]);
        Row[1] = detail::scale(Row[1], static_cast<T>(1));


        // Next, get Z scale and normalize 3rd row.
        outScale.value.z = length(Row[2]);
        Row[2] = detail::scale(Row[2], static_cast<T>(1));


        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.

        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                outScale.value[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }


        outRotation.value.y = asin(-Row[0][2]);
        if (cos(outRotation.value.y) != 0)
        {
            outRotation.value.x = atan2(Row[1][2], Row[2][2]);
            outRotation.value.z = atan2(Row[0][1], Row[0][0]);
        }
        else
        {
            outRotation.value.x = atan2(-Row[2][0], Row[1][1]);
            outRotation.value.z = 0;
        }

        return true;
    }
    Vector3 Math::EulerAngles(Quaternion quat)
    {
        return Vector3(glm::eulerAngles(quat.value));
    }

    float Math::Length(const Vector2& v)
    {
        return glm::length(v.value);
    }

    float Math::Length(const Vector3& v)
    {
        return glm::length(v.value);
    }

    float Math::Length(const Vector4& v)
    {
        return glm::length(v.value);
    }

    float Math::Max(const Vector2& v)
    {
        return glm::max(v.value.x, v.value.y);
    }

    float Math::Max(const Vector3& v)
    {
        return glm::max(v.value.x, v.value.y, v.value.z);
    }

    float Math::Max(const Vector4& v)
    {
        return glm::max(v.value.x, v.value.y, v.value.z, v.value.w);
    }

    float Math::Dot(const Vector2& v1, const Vector2& v2)
    {
        return glm::dot(v1.value, v2.value);
    }

    float Math::Dot(const Vector3& v1, const Vector3& v2)
    {
        return glm::dot(v1.value, v2.value);
    }

    float Math::Dot(const Vector4& v1, const Vector4& v2)
    {
        return glm::dot(v1.value, v2.value);
    }

    Vector3 Math::Clamp(const Vector3& value, const Vector3& min, const Vector3& max)
    {
        return glm::clamp(value.value, min.value, max.value);
    }

    float Math::Clamp(float value, float min, float max)
    {
        return glm::clamp(value, min, max);
    }

    Vector3 Math::Reflect(const Vector3& v, const Vector3& normal)
    {
        return glm::reflect(v.value, normal.value);
    }

    float Math::Sqrt(float value)
    {
        return glm::sqrt(value);    
    }

    float Math::Tan(float angle)
    {
        return glm::tan(angle);
    }

    Matrix4x4 Math::IdentityMatrix()
    {
        return Matrix4x4(1.0f);
    }

    float Math::Sin(float angle)
    {
        return glm::sin(angle);
    }

    float Math::Cos(float angle)
    {
        return glm::cos(angle);
    }

}
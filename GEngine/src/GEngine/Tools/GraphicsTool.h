#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API GraphicsTool
	{
	public:
		static void Init();

		static void RenderPoint(const Vector3& pos, const Vector4& color = Vector4(1.0f, 1.0f, 1.0f, 1.0f), const float& size = 1.0f);
		static void RenderLine(const Vector3& start, const Vector3& end, const Vector4& color = Vector4(1.0f, 1.0f, 1.0f, 1.0f), const float& width = 1.0f);
	};
}



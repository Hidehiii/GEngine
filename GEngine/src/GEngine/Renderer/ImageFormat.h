#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum class RenderImage2DFormat
	{
		RGBA8F,
		RGB8F,
	};

	enum class ComputeImage2DFormat
	{
		RGBA32F,
		RGBA16F,
		RG32F,
		RG16F,
		R32F,
		R16F,
		RGBA32I,
		RGBA16I,
		RGBA8I,
		RGBA32UI,
		RGBA16UI,
		RGBA8UI
	};
}
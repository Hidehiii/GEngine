#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	enum class RenderImage2DFormat
	{
		None,
		RGBA8F,
		RGB8F,
	};

	static uint32_t RenderImage2DFormatChannelSize(RenderImage2DFormat format)
	{
		switch (format)
		{
		case RenderImage2DFormat::None: return 0;
		case RenderImage2DFormat::RGBA8F: return 4;
		case RenderImage2DFormat::RGB8F: return 3;
		default:
			break;
		}
	}

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
		RGBA8UI,
		R32UI
	};
}
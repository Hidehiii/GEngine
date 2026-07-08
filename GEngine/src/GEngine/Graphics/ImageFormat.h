#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
	

	static uint32_t RenderImage2DFormatChannelSize(RenderImage2DFormat format)
	{
		switch (format)
		{
		case RENDER_IMAGE_2D_FORMAT_NONE:		return 0;
		case RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM:		return 4;
		case RENDER_IMAGE_2D_FORMAT_RGBA8_SNORM:		return 4;
		case RENDER_IMAGE_2D_FORMAT_RGB8_UNORM:		return 3;
		case RENDER_IMAGE_2D_FORMAT_RGB8_SNORM:		return 3;
		default:
			break;
		}
	}

	
}
#include "GEpch.h"
#include "GEngine/Tools/GraphicsTool.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include <stdexcept>

namespace GEngine
{
	struct GraphicsToolData
	{
		Ref<GraphicsPipeline>	LinePipeline;
		Ref<GraphicsPipeline>	PointPipeline;
	};

	void GraphicsTool::Init()
	{
		throw std::logic_error("GraphicsTool point and line rendering is unsupported.");
	}

	void GraphicsTool::RenderPoint(const Vector3& pos, const Vector4& color, const float& size)
	{
		throw std::logic_error("GraphicsTool::RenderPoint is unsupported.");
	}

	void GraphicsTool::RenderLine(const Vector3& start, const Vector3& end, const Vector4& color, const float& width)
	{
		throw std::logic_error("GraphicsTool::RenderLine is unsupported.");
	}
}


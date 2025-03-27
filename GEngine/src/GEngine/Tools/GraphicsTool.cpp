#include "GEpch.h"
#include "GraphicsTool.h"
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsPipeline.h"

namespace GEngine
{
	struct GraphicsToolData
	{
		Ref<GraphicsPipeline>	LinePipeline;
		Ref<GraphicsPipeline>	PointPipeline;
	};

	void GraphicsTool::Init()
	{
	}

	void GraphicsTool::RenderPoint(const Vector3& pos, const Vector4& color, const float& size)
	{
	}

	void GraphicsTool::RenderLine(const Vector3& start, const Vector3& end, const Vector4& color, const float& width)
	{
	}
}


#include "GEpch.h"
#include "GEngine/Layer.h"

namespace GEngine
{
	Layer::Layer(const std::string& debugName)
		: m_Name(debugName)
	{
	}

	Layer::~Layer()
	{
	}
}

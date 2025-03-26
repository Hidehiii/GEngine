#include "GEpch.h"
#include "LayerStack.h"

namespace GEngine
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.push_back(layer);
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	Layer* LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			(*it)->OnDetach();
			return *it;
		}
		return nullptr;
	}

	Layer* LayerStack::PopOverlay()
	{
		Layer* layer = nullptr;
		if (m_Layers.size() > 0)
		{
			layer = *m_Layers.rbegin();
			m_Layers.erase(m_Layers.begin() + m_Layers.size() - 1);
			layer->OnDetach();
		}
		return layer;
	}
}

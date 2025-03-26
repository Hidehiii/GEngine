#pragma once

#include "GEngine/Core/Core.h"
#include "Layer.h"

namespace GEngine
{
	class GENGINE_API LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		Layer* PopLayer(Layer* layer);

		void PushOverlay(Layer* overlay);
		Layer* PopOverlay();

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }

	private:
		std::vector<Layer*> m_Layers;
	};
}



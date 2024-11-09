#pragma once
#include <vector>
#include "Epoch/Debug/Assert.h"
#include "Layer.h"

namespace Epoch
{
	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		void PushLayer(Layer* aLayer); 
		void PushOverlay(Layer* aOverlay); 

		Layer* operator[](size_t index)
		{
			EPOCH_ASSERT(index >= 0 && index < myLayers.size(), "Out of scope!");
			return myLayers[index];
		}

		const Layer* operator[](size_t index) const
		{
			EPOCH_ASSERT(index >= 0 && index < myLayers.size(), "Out of scope!");
			return myLayers[index];
		}

		size_t Size() const { return myLayers.size(); }

		std::vector<Layer*>::iterator begin() { return myLayers.begin(); }
		std::vector<Layer*>::iterator end() { return myLayers.end(); }

	private:
		std::vector<Layer*> myLayers;
		unsigned int myLayerInsertIndex = 0;
	};
}

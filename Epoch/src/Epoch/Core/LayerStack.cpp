#include "epch.h"
#include "LayerStack.h"

namespace Epoch
{
	LayerStack::~LayerStack()
	{
	}

	void LayerStack::PushLayer(Layer* aLayer)
	{
		myLayers.emplace(myLayers.begin() + myLayerInsertIndex, aLayer);
		myLayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* aOverlay)
	{
		myLayers.emplace_back(aOverlay);
	}
}

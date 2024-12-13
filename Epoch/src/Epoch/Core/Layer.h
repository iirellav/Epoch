#pragma once
#include <string>
#include "Events/Event.h"

namespace Epoch
{
	class Layer
	{
	public:
		Layer(const std::string& aName = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& aEvent) {}

		const std::string& GetName() const { return myDebugName; }

	private:
		const std::string myDebugName;
	};
}

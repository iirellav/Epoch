#pragma once
#include <string>

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

		const std::string& GetName() const { return myDebugName; }

	private:
		const std::string myDebugName;
	};
}

#pragma once
#include "Epoch/Core/Layer.h"

namespace Epoch
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void Begin();
		void End();

		void SetDarkThemeColors();
	};
}

#pragma once
#include <vector>
#include <CommonUtilities/Gradient.h>
#include "Epoch/ImGui/ImGuiUtilities.h"

namespace Epoch
{
	class GradientEditor
	{
	public:
		static GradientEditor& Get() { static GradientEditor instance; return instance; }
		void Init() { DeserializePresets(); }

		bool OnImGuiRender();

		void SetGradientToEdit(CU::Gradient* aGradient);
		bool IsOpen() const { return myIsOpen; }
		void Close();

	private:
		GradientEditor() = default;
		~GradientEditor() = default;

		void DrawGradientColorKeys(ImVec2 aBarPos, float aMaxWidth);
		void DrawGradientAlphaKeys(ImVec2 aBarPos, float aMaxWidth);
		
		void SerializePresets();
		void DeserializePresets();

	private:
		bool myIsOpen = false;
		bool mySetFocus = false;

		bool myDraggingKey = false;
		int mySelectedPresetIndex = -1;

		std::shared_ptr<CU::Gradient::Key> mySelectedKey = nullptr;
		CU::Gradient* myGradient = nullptr;

		std::vector<CU::Gradient> myPresets;
	};
}

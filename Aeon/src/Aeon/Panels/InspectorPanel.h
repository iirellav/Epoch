#pragma once
#include <functional>
#include <unordered_map>
#include <Epoch/Assets/AssetTypes.h>
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class InspectorPanel : public EditorPanel
	{
	public:
		InspectorPanel();
		~InspectorPanel() = default;
		
		void OnImGuiRender(bool& aIsOpen) override;

	private:
		void DrawMaterialInspector(UUID aAssetID);
		void DrawMeshInspector(UUID aAssetID);
		void DrawPrefabInspector(UUID aAssetID);
		
	private:
		std::unordered_map<AssetType, std::function<void(UUID aAssetID)>> myDrawFunctions;
	};
}

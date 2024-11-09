#pragma once
#include <string>
#include <CommonUtilities/Math/Vector/Vector.h>

namespace Epoch
{
	enum class AxisOrientationMode { Local, World };
	enum class TransformationTarget { MedianPoint, IndividualOrigins };
	enum class GridPlane { X, Y, Z };
	enum class ReloadScriptAssemblyWhilePlaying { Stop, Wait };

	struct EditorSettings
	{
		//---------- General ------------
		bool loadLastOpenProject = false;
		std::string lastProjectPath = "";
		bool autoSaveSceneBeforePlay = true;

		//---------- Level Editor ------------
		float translationSnapValue = 50.0f;
		float rotationSnapValue = 45.0f;
		float scaleSnapValue = 0.5f;

		AxisOrientationMode axisOrientationMode = AxisOrientationMode::Local;
		TransformationTarget multiTransformTarget = TransformationTarget::MedianPoint;

		bool createEntitiesAtOrigin = true;

		bool gridEnabled = true;
		float gridOpacity = 0.5f;
		CU::Vector2i gridSize = { 20, 20 };
		GridPlane gridPlane = GridPlane::Y;

		//---------- Content Browser ------------
		float contentBrowserThumbnailSize = 100.0f;

		//---------- Auto Save ------------
		bool autosaveEnabled = true;
		int autosaveIntervalSeconds = 300;

		//---------- Scripting ------------
		bool automaticallyReloadScriptAssembly = true;
		ReloadScriptAssemblyWhilePlaying reloadScriptAssemblyWhilePlaying = ReloadScriptAssemblyWhilePlaying::Stop;
		bool clearConsoleOnPlay = true;
		bool collapseConsoleMessages = true;


		static EditorSettings& Get() { static EditorSettings staticSettings; return staticSettings; }
	};

	class EditorSettingsSerializer
	{
	public:
		static bool Init();

		static void SaveSettings();

	private:
		static bool LoadSettings();
	};
}

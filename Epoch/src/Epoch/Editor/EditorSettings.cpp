#include "epch.h"
#include "EditorSettings.h"
#include <filesystem>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Instrumentor.h>
#include <Epoch/Utils/YAMLSerializationHelpers.h>

namespace Epoch
{
	static std::filesystem::path staticEditorSettingsPath;

	bool EditorSettingsSerializer::Init()
	{
		EPOCH_PROFILE_FUNC();

		staticEditorSettingsPath = std::filesystem::absolute("Config");
		if (!std::filesystem::exists(staticEditorSettingsPath)) std::filesystem::create_directory(staticEditorSettingsPath);
		staticEditorSettingsPath /= "EditorSettings.config";

		return LoadSettings();
	}

	bool EditorSettingsSerializer::LoadSettings()
	{
		if (!std::filesystem::exists(staticEditorSettingsPath))
		{
			SaveSettings();
			return true;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(staticEditorSettingsPath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR("Failed to load .config file '{}'\n     {}", staticEditorSettingsPath.string(), e.what());
			return false;
		}

		if (!data["EditorSettings"])
		{
			return false;
		}

		YAML::Node rootNode = data["EditorSettings"];

		auto& settings = EditorSettings::Get();
		
		//---------- General ------------
		settings.loadLastOpenProject = rootNode["LoadLastOpenProject"].as<bool>(false);
		settings.lastProjectPath = rootNode["LastProjectPath"].as<std::string>(std::string());
		settings.autoSaveSceneBeforePlay = rootNode["AutoSaveSceneBeforePlay"].as<bool>(true);
		
		//---------- Level Editor ------------
		settings.translationSnapValue = rootNode["TranslationSnapValue"].as<float>(0.5f);
		settings.rotationSnapValue = rootNode["RotationSnapValue"].as<float>(45.0f);
		settings.scaleSnapValue = rootNode["ScaleSnapValue"].as<float>(0.5f);

		settings.axisOrientationMode = (AxisOrientationMode)rootNode["AxisOrientation"].as<int>((int)AxisOrientationMode::Local);
		settings.multiTransformTarget = (TransformationTarget)rootNode["MultiTransformTarget"].as<int>((int)TransformationTarget::MedianPoint);

		settings.createEntitiesAtOrigin = rootNode["CreateEntitiesAtOrigin"].as<bool>(true);

		settings.gridEnabled = rootNode["GridEnabled"].as<bool>(true);
		settings.gridOpacity = rootNode["GridOpacity"].as<float>(0.5f);
		settings.gridSize.x = rootNode["GridSizeX"].as<int>(20);
		settings.gridSize.y = rootNode["GridSizeY"].as<int>(20);
		settings.gridPlane = (GridPlane)rootNode["GridPlane"].as<int>((int)GridPlane::Y);

		//---------- Content Browser ------------
		settings.contentBrowserThumbnailSize = rootNode["ContentBrowserThumbnailSize"].as<float>(100.0f);

		//---------- Auto Save ------------
		settings.autosaveEnabled = rootNode["AutosaveEnabled"].as<bool>(true);
		settings.autosaveIntervalSeconds = rootNode["AutosaveIntervalSeconds"].as<int>(300);
		
		//---------- Scripting ------------
		settings.automaticallyReloadScriptAssembly = rootNode["AutomaticallyReloadScriptAssembly"].as<bool>(true);
		settings.reloadScriptAssemblyWhilePlaying = (ReloadScriptAssemblyWhilePlaying)rootNode["ReloadScriptAssemblyWhilePlaying"].as<int>((int)ReloadScriptAssemblyWhilePlaying::Stop);
		settings.clearConsoleOnPlay = rootNode["ClearConsoleOnPlay"].as<bool>(true);
		settings.collapseConsoleMessages = rootNode["CollapseConsoleMessages"].as<bool>(true);

		return true;
	}

	void EditorSettingsSerializer::SaveSettings()
	{
		const auto& settings = EditorSettings::Get();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "EditorSettings" << YAML::Value << YAML::BeginMap;
		
		//---------- General ------------
		out << YAML::Key << "LoadLastOpenProject" << YAML::Value << settings.loadLastOpenProject;
		out << YAML::Key << "LastProjectPath" << YAML::Value << settings.lastProjectPath;
		out << YAML::Key << "AutoSaveSceneBeforePlay" << YAML::Value << settings.autoSaveSceneBeforePlay;

		//---------- Level Editor ------------
		out << YAML::Key << "TranslationSnapValue" << YAML::Value << settings.translationSnapValue;
		out << YAML::Key << "RotationSnapValue" << YAML::Value << settings.rotationSnapValue;
		out << YAML::Key << "ScaleSnapValue" << YAML::Value << settings.scaleSnapValue;

		out << YAML::Key << "AxisOrientation" << YAML::Value << (int)settings.axisOrientationMode;
		out << YAML::Key << "MultiTransformTarget" << YAML::Value << (int)settings.multiTransformTarget;

		out << YAML::Key << "CreateEntitiesAtOrigin" << YAML::Value << settings.createEntitiesAtOrigin;

		out << YAML::Key << "GridEnabled" << YAML::Value << settings.gridEnabled;
		out << YAML::Key << "GridOpacity" << YAML::Value << settings.gridOpacity;
		out << YAML::Key << "GridSizeX" << YAML::Value << settings.gridSize.x;
		out << YAML::Key << "GridSizeY" << YAML::Value << settings.gridSize.y;
		out << YAML::Key << "GridPlane" << YAML::Value << (int)settings.gridPlane;

		//---------- Content Browser ------------
		out << YAML::Key << "ContentBrowserThumbnailSize" << YAML::Value << settings.contentBrowserThumbnailSize;

		//---------- Auto Save ------------
		out << YAML::Key << "AutosaveEnabled" << YAML::Value << settings.autosaveEnabled;
		out << YAML::Key << "AutosaveIntervalSeconds" << YAML::Value << settings.autosaveIntervalSeconds;
		
		//---------- Scripting ------------
		out << YAML::Key << "AutomaticallyReloadScriptAssembly" << YAML::Value << settings.automaticallyReloadScriptAssembly;
		out << YAML::Key << "ReloadScriptAssemblyWhilePlaying" << YAML::Value << (int)settings.reloadScriptAssemblyWhilePlaying;
		out << YAML::Key << "ClearConsoleOnPlay" << YAML::Value << settings.clearConsoleOnPlay;
		out << YAML::Key << "CollapseConsoleMessages" << YAML::Value << settings.collapseConsoleMessages;

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::ofstream fout(staticEditorSettingsPath);
		fout << out.c_str();
		fout.close();
	}
}

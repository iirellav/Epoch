#include "epch.h"
#include "ProjectSerializer.h"
#include "Epoch/Physics/PhysicsSystem.h"
#pragma warning(push, 0)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)

namespace Epoch
{
	void ProjectSerializer::Serialize(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		auto& config = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << config.name;
			out << YAML::Key << "ProductName" << YAML::Value << config.productName;
			out << YAML::Key << "CompanyName" << YAML::Value << config.companyName;
			out << YAML::Key << "Version" << YAML::Value << config.version;

			out << YAML::Key << "StartScene" << YAML::Value << config.startScene.string();
			out << YAML::Key << "EditorCameraPosition" << YAML::Value << config.editorCameraPosition;
			out << YAML::Key << "EditorCameraRotation" << YAML::Value << config.editorCameraRotation;

			out << YAML::Key << "AutosaveDirectory" << YAML::Value << config.autosaveDirectory.string();

			out << YAML::Key << "AssetDirectory" << YAML::Value << config.assetDirectory.string();
			out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.assetRegistryPath.string();

			out << YAML::Key << "DefaultScriptNamespace" << YAML::Value << config.defaultScriptNamespace;

			out << YAML::Key << "FixedTimestep" << YAML::Value << physicsSettings.fixedTimestep;
			out << YAML::Key << "Gravity" << YAML::Value << physicsSettings.gravity;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		LOG_DEBUG("Serializing project '{}'", config.name);

		std::ofstream fileOut(aFilepath);
		fileOut << out.c_str();
		fileOut.close();
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(aFilepath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR("Failed to load .epoch file '{}'\n     {}", aFilepath.string(), e.what());
			return false;
		}

		if (!data["Project"])
		{
			return false;
		}


		YAML::Node rootNode = data["Project"];

		auto& aConfig = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		aConfig.name = rootNode["Name"].as<std::string>();
		LOG_DEBUG("Deserializing project '{}'", myProject->myConfig.name);
		aConfig.productName = rootNode["ProductName"].as<std::string>(aConfig.name);

		const std::filesystem::path& projectPath = aFilepath;
		aConfig.projectFileName = projectPath.filename().string();
		aConfig.projectDirectory = projectPath.parent_path().string();
		
		aConfig.companyName = rootNode["CompanyName"].as<std::string>("");
		aConfig.version = rootNode["Version"].as<float>(0.1f);

		aConfig.startScene = rootNode["StartScene"].as<std::string>("");
		aConfig.editorCameraPosition = rootNode["EditorCameraPosition"].as<CU::Vector3f>(CU::Vector3f::Zero);
		aConfig.editorCameraRotation = rootNode["EditorCameraRotation"].as<CU::Vector3f>(CU::Vector3f::Zero);

		aConfig.autosaveDirectory = rootNode["AutosaveDirectory"].as<std::string>("Autosaves");
		if (!std::filesystem::exists(aConfig.projectDirectory / aConfig.autosaveDirectory))
		{
			LOG_WARNING("Autosave directory does not exist!");
			LOG_INFO("Autosave directory created");
			std::filesystem::create_directory(aConfig.projectDirectory / aConfig.autosaveDirectory);
		}

		aConfig.assetDirectory = rootNode["AssetDirectory"].as<std::string>("Assets");
		if (!std::filesystem::exists(aConfig.projectDirectory / aConfig.assetDirectory))
		{
			LOG_WARNING("Asset directory does not exist!");
			LOG_INFO("Asset directory created");
			std::filesystem::create_directory(aConfig.projectDirectory / aConfig.assetDirectory);
		}

		aConfig.assetRegistryPath = rootNode["AssetRegistryPath"].as<std::string>(aConfig.assetRegistryPath.string());
		
		aConfig.defaultScriptNamespace = rootNode["DefaultScriptNamespace"].as<std::string>(aConfig.projectFileName.stem().string());
		aConfig.defaultScriptNamespace = CU::RemoveWhitespaces(aConfig.defaultScriptNamespace);

		physicsSettings.fixedTimestep = rootNode["FixedTimestep"].as<float>(1.0f/60.0f);
		physicsSettings.gravity = rootNode["Gravity"].as<CU::Vector3f>(CU::Vector3f(0.0f, -982.0f, 0.0f));

		return true;
	}

	void ProjectSerializer::SerializeRuntime(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		auto& aConfig = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << aConfig.name;
			out << YAML::Key << "ProductName" << YAML::Value << aConfig.productName;
			out << YAML::Key << "CompanyName" << YAML::Value << aConfig.companyName;
			out << YAML::Key << "Version" << YAML::Value << aConfig.version;
			
			out << YAML::Key << "FixedTimestep" << YAML::Value << physicsSettings.fixedTimestep;
			out << YAML::Key << "Gravity" << YAML::Value << physicsSettings.gravity;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		LOG_DEBUG("Serializing runtime project '{}'", aConfig.name);

		std::ofstream fout(aFilepath);
		fout << out.c_str();
		fout.close();
	}

	bool ProjectSerializer::DeserializeRuntime(const std::filesystem::path& aFilepath)
	{
		EPOCH_PROFILE_FUNC();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(aFilepath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR("Failed to load .epoch file '{}'\n     {}", aFilepath.string(), e.what());
			return false;
		}

		if (!data["Project"])
		{
			return false;
		}


		YAML::Node rootNode = data["Project"];

		auto& aConfig = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		aConfig.name = rootNode["Name"].as<std::string>();
		LOG_DEBUG("Deserializing project '{}'", myProject->myConfig.name);
		aConfig.productName = rootNode["ProductName"].as<std::string>(aConfig.name);

		const std::filesystem::path& projectPath = aFilepath;
		aConfig.projectFileName = projectPath.filename().string();
		aConfig.projectDirectory = projectPath.parent_path().string();
		
		aConfig.companyName = rootNode["CompanyName"].as<std::string>("");
		aConfig.version = rootNode["Version"].as<float>(0.1f);

		physicsSettings.fixedTimestep = rootNode["FixedTimestep"].as<float>(1.0f/60.0f);
		physicsSettings.gravity = rootNode["Gravity"].as<CU::Vector3f>(CU::Vector3f(0.0f, -982.0f, 0.0f));

		return true;
	}
}

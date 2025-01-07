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

			out << YAML::Key << "RuntimeStartScene" << YAML::Value << config.runtimeStartScene;

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

		auto& config = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		config.name = rootNode["Name"].as<std::string>();
		LOG_DEBUG("Deserializing project '{}'", myProject->myConfig.name);
		config.productName = rootNode["ProductName"].as<std::string>(config.name);

		const std::filesystem::path& projectPath = aFilepath;
		config.projectFileName = projectPath.filename().string();
		config.projectDirectory = projectPath.parent_path().string();
		
		config.companyName = rootNode["CompanyName"].as<std::string>("");
		config.version = rootNode["Version"].as<std::string>(config.version);

		config.startScene = rootNode["StartScene"].as<std::string>("");
		config.editorCameraPosition = rootNode["EditorCameraPosition"].as<CU::Vector3f>(CU::Vector3f::Zero);
		config.editorCameraRotation = rootNode["EditorCameraRotation"].as<CU::Vector3f>(CU::Vector3f::Zero);

		config.runtimeStartScene = rootNode["RuntimeStartScene"].as<UUID>(UUID(0));

		config.autosaveDirectory = rootNode["AutosaveDirectory"].as<std::string>("Autosaves");
		if (!std::filesystem::exists(config.projectDirectory / config.autosaveDirectory))
		{
			LOG_WARNING("Autosave directory does not exist!");
			LOG_INFO("Autosave directory created");
			std::filesystem::create_directory(config.projectDirectory / config.autosaveDirectory);
		}

		config.assetDirectory = rootNode["AssetDirectory"].as<std::string>("Assets");
		if (!std::filesystem::exists(config.projectDirectory / config.assetDirectory))
		{
			LOG_WARNING("Asset directory does not exist!");
			LOG_INFO("Asset directory created");
			std::filesystem::create_directory(config.projectDirectory / config.assetDirectory);
		}

		config.assetRegistryPath = rootNode["AssetRegistryPath"].as<std::string>(config.assetRegistryPath.string());
		
		config.defaultScriptNamespace = rootNode["DefaultScriptNamespace"].as<std::string>(config.projectFileName.stem().string());
		config.defaultScriptNamespace = CU::RemoveWhitespaces(config.defaultScriptNamespace);

		physicsSettings.fixedTimestep = rootNode["FixedTimestep"].as<float>(1.0f/60.0f);
		physicsSettings.gravity = rootNode["Gravity"].as<CU::Vector3f>(CU::Vector3f(0.0f, -982.0f, 0.0f));

		return true;
	}

	void ProjectSerializer::SerializeRuntime(const std::filesystem::path& aFilepath)
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

			out << YAML::Key << "RuntimeStartScene" << YAML::Value << config.runtimeStartScene;
			
			out << YAML::Key << "FixedTimestep" << YAML::Value << physicsSettings.fixedTimestep;
			out << YAML::Key << "Gravity" << YAML::Value << physicsSettings.gravity;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		LOG_DEBUG("Serializing runtime project '{}'", config.name);

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

		auto& config = myProject->myConfig;
		auto& physicsSettings = PhysicsSystem::GetSettings();

		config.name = rootNode["Name"].as<std::string>();
		LOG_DEBUG("Deserializing project '{}'", myProject->myConfig.name);
		config.productName = rootNode["ProductName"].as<std::string>(config.name);

		const std::filesystem::path& projectPath = aFilepath;
		config.projectFileName = projectPath.filename().string();
		config.projectDirectory = projectPath.parent_path().string();
		
		config.companyName = rootNode["CompanyName"].as<std::string>("");
		config.version = rootNode["Version"].as<std::string>(config.version);

		config.runtimeStartScene = rootNode["RuntimeStartScene"].as<UUID>(UUID(0));
		
		physicsSettings.fixedTimestep = rootNode["FixedTimestep"].as<float>(1.0f/60.0f);
		physicsSettings.gravity = rootNode["Gravity"].as<CU::Vector3f>(CU::Vector3f(0.0f, -982.0f, 0.0f));

		return true;
	}
}

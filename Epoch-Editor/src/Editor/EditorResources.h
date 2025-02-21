#pragma once
#include <memory>
#include <filesystem>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Profiler.h>
#include <Epoch/Core/Application.h>
#include <Epoch/Rendering/Texture.h>

namespace Epoch
{
	class EditorResources
	{
	public:
		//Generic
		inline static std::shared_ptr<Texture2D> GearIcon = nullptr;
		inline static std::shared_ptr<Texture2D> VerticalEllipsisIcon = nullptr;
		inline static std::shared_ptr<Texture2D> QuestionMarkIcon = nullptr;
		inline static std::shared_ptr<Texture2D> ForwardIcon = nullptr;
		inline static std::shared_ptr<Texture2D> BackIcon = nullptr;
		inline static std::shared_ptr<Texture2D> RefreshIcon = nullptr;

		// Toolbar
		inline static std::shared_ptr<Texture2D> PlayButton = nullptr;
		inline static std::shared_ptr<Texture2D> StopButton = nullptr;
		inline static std::shared_ptr<Texture2D> PauseButton = nullptr;
		inline static std::shared_ptr<Texture2D> SimulateButton = nullptr;
		inline static std::shared_ptr<Texture2D> StepButton = nullptr;

		// Content browser
		inline static std::shared_ptr<Texture2D> ClosedFolderIcon = nullptr;
		inline static std::shared_ptr<Texture2D> OpenFolderIcon = nullptr;
		inline static std::shared_ptr<Texture2D> ModelIcon = nullptr;
		inline static std::shared_ptr<Texture2D> TextureIcon = nullptr;
		inline static std::shared_ptr<Texture2D> VideoIcon = nullptr;
		inline static std::shared_ptr<Texture2D> SceneIcon = nullptr;
		inline static std::shared_ptr<Texture2D> ScriptFileIcon = nullptr;
		inline static std::shared_ptr<Texture2D> PrefabIcon = nullptr;
		inline static std::shared_ptr<Texture2D> MaterialIcon = nullptr;
		inline static std::shared_ptr<Texture2D> FontIcon = nullptr;
		inline static std::shared_ptr<Texture2D> OtherIcon = nullptr;

		// Components
		inline static std::shared_ptr<Texture2D> TransformIcon = nullptr;
		inline static std::shared_ptr<Texture2D> MeshRendererIcon = nullptr;
		inline static std::shared_ptr<Texture2D> SpriteRendererIcon = nullptr;
		inline static std::shared_ptr<Texture2D> TextRendererIcon = nullptr;
		inline static std::shared_ptr<Texture2D> CameraIcon = nullptr;
		inline static std::shared_ptr<Texture2D> SkyLightIcon = nullptr;
		inline static std::shared_ptr<Texture2D> DirectionalLightIcon = nullptr;
		inline static std::shared_ptr<Texture2D> SpotlightIcon = nullptr;
		inline static std::shared_ptr<Texture2D> PointLightIcon = nullptr;
		inline static std::shared_ptr<Texture2D> ParticleSystemIcon = nullptr;
		inline static std::shared_ptr<Texture2D> RigidbodyIcon = nullptr;
		inline static std::shared_ptr<Texture2D> BoxColliderIcon = nullptr;
		inline static std::shared_ptr<Texture2D> SphereColliderIcon = nullptr;
		inline static std::shared_ptr<Texture2D> CapsuleColliderIcon = nullptr;
		inline static std::shared_ptr<Texture2D> CharacterControllerIcon = nullptr;
		inline static std::shared_ptr<Texture2D> ScriptIcon = nullptr;

		static void Init()
		{
			EPOCH_PROFILE_FUNC();

			JobSystem& js = Application::Get().GetJobSystem();

			//Generic
			js.AddAJob(NewLoadTexture, &GearIcon,					"Generic/GearIcon.png");
			js.AddAJob(NewLoadTexture, &VerticalEllipsisIcon,		"Generic/VerticalEllipsisIcon.png");
			js.AddAJob(NewLoadTexture, &QuestionMarkIcon,			"Generic/QuestionMarkIcon.png");
			js.AddAJob(NewLoadTexture, &ForwardIcon,				"Generic/ForwardIcon.png");
			js.AddAJob(NewLoadTexture, &BackIcon,					"Generic/BackIcon.png");
			js.AddAJob(NewLoadTexture, &RefreshIcon,				"Generic/RefreshIcon.png");

			// Toolbar
			js.AddAJob(NewLoadTexture, &PlayButton,					"Toolbar/PlayButton.png");
			js.AddAJob(NewLoadTexture, &StopButton,					"Toolbar/StopButton.png");
			js.AddAJob(NewLoadTexture, &PauseButton,				"Toolbar/PauseButton.png");
			js.AddAJob(NewLoadTexture, &SimulateButton,				"Toolbar/SimulateButton.png");
			js.AddAJob(NewLoadTexture, &StepButton,					"Toolbar/StepButton.png");

			// Content browser
			js.AddAJob(NewLoadTexture, &ClosedFolderIcon,			"ContentBrowser/DirectoryIconClosed.png");
			js.AddAJob(NewLoadTexture, &OpenFolderIcon,				"ContentBrowser/DirectoryIconOpen.png");
			js.AddAJob(NewLoadTexture, &ModelIcon,					"ContentBrowser/ModelIcon.png");
			js.AddAJob(NewLoadTexture, &TextureIcon,				"ContentBrowser/TextureIcon.png");
			js.AddAJob(NewLoadTexture, &VideoIcon,					"ContentBrowser/VideoIcon.png");
			js.AddAJob(NewLoadTexture, &SceneIcon,					"ContentBrowser/SceneIcon.png");
			js.AddAJob(NewLoadTexture, &ScriptFileIcon,				"ContentBrowser/ScriptFileIcon.png");
			js.AddAJob(NewLoadTexture, &PrefabIcon,					"ContentBrowser/PrefabIcon.png");
			js.AddAJob(NewLoadTexture, &MaterialIcon,				"ContentBrowser/MaterialIcon.png");
			js.AddAJob(NewLoadTexture, &FontIcon,					"ContentBrowser/FontIcon.png");
			js.AddAJob(NewLoadTexture, &OtherIcon,					"ContentBrowser/FileIcon.png");

			// Components
			js.AddAJob(NewLoadTexture, &TransformIcon,				"Components/Transform.png");
			js.AddAJob(NewLoadTexture, &MeshRendererIcon,			"Components/MeshRenderer.png");
			js.AddAJob(NewLoadTexture, &SpriteRendererIcon,			"Components/SpriteRenderer.png");
			js.AddAJob(NewLoadTexture, &TextRendererIcon,			"Components/TextRenderer.png");
			js.AddAJob(NewLoadTexture, &CameraIcon,					"Components/Camera.png");
			js.AddAJob(NewLoadTexture, &SkyLightIcon,				"Components/DirectionalLight.png");
			js.AddAJob(NewLoadTexture, &DirectionalLightIcon,		"Components/DirectionalLight.png");
			js.AddAJob(NewLoadTexture, &SpotlightIcon,				"Components/Spotlight.png");
			js.AddAJob(NewLoadTexture, &PointLightIcon,				"Components/PointLight.png");
			js.AddAJob(NewLoadTexture, &RigidbodyIcon,				"Components/RigidBody.png");
			js.AddAJob(NewLoadTexture, &BoxColliderIcon,			"Components/BoxCollider.png");
			js.AddAJob(NewLoadTexture, &SphereColliderIcon,			"Components/SphereCollider.png");
			js.AddAJob(NewLoadTexture, &CapsuleColliderIcon,		"Components/CapsuleCollider.png");
			js.AddAJob(NewLoadTexture, &CharacterControllerIcon,	"Components/CharacterController.png");
			js.AddAJob(NewLoadTexture, &ScriptIcon,					"Components/Script.png");



			////Generic
			//GearIcon =					LoadTexture("Generic/GearIcon.png");
			//VerticalEllipsisIcon =		LoadTexture("Generic/VerticalEllipsisIcon.png");
			//QuestionMarkIcon =			LoadTexture("Generic/QuestionMarkIcon.png");
			//
			//// Toolbar
			//PlayButton =					LoadTexture("Toolbar/PlayButton.png");
			//StopButton =					LoadTexture("Toolbar/StopButton.png");
			//PauseButton =					LoadTexture("Toolbar/PauseButton.png");
			//SimulateButton =				LoadTexture("Toolbar/SimulateButton.png");
			//StepButton =					LoadTexture("Toolbar/StepButton.png");
			//
			//// Content browser
			//DirectoryIcon =				LoadTexture("ContentBrowser/DirectoryIcon.png");
			//ModelIcon =					LoadTexture("ContentBrowser/ModelIcon.png");
			//TextureIcon =					LoadTexture("ContentBrowser/TextureIcon.png");
			//SceneIcon =					LoadTexture("ContentBrowser/epoFileIcon.png");
			//OtherIcon =					LoadTexture("ContentBrowser/FileIcon.png");
			//
			//// Components
			//TransformIcon =				LoadTexture("Components/Transform.png");
			//MeshRendererIcon =			LoadTexture("Components/MeshRenderer.png");
			//SpriteRendererIcon =			LoadTexture("Components/SpriteRenderer.png");
			//TextRendererIcon =			LoadTexture("Components/TextRenderer.png");
			//CameraIcon =					LoadTexture("Components/Camera.png");
			//SkyLightIcon =				LoadTexture("Components/DirectionalLight.png");
			//DirectionalLightIcon =		LoadTexture("Components/DirectionalLight.png");
			//SpotlightIcon =				LoadTexture("Components/Spotlight.png");
			//PointLightIcon =				LoadTexture("Components/PointLight.png");
			//RigidbodyIcon =				LoadTexture("Components/RigidBody.png");
			//BoxColliderIcon =				LoadTexture("Components/BoxCollider.png");
			//SphereColliderIcon =			LoadTexture("Components/SphereCollider.png");
			//CapsuleColliderIcon =			LoadTexture("Components/CapsuleCollider.png");
			//CharacterControllerIcon =		LoadTexture("Components/CharacterController.png");
		}

	private:
		static std::shared_ptr<Texture2D> LoadTexture(const std::filesystem::path& aPath)
		{
			std::filesystem::path path = std::filesystem::path("Resources/Icons") / aPath;

			if (!std::filesystem::exists(path))
			{
				const std::string msg = std::format("Failed to load icon {}! The file doesn't exist!", path.string());
				EPOCH_ASSERT(false, msg);
				return nullptr;
			}

			std::shared_ptr<Texture2D> texture = Texture2D::Create(path);
			return texture;
		}

		static void NewLoadTexture(std::shared_ptr<Texture2D>* aTexture, const std::filesystem::path& aPath)
		{
			std::filesystem::path path = std::filesystem::path("Resources/Icons") / aPath;

			if (!std::filesystem::exists(path))
			{
				const std::string msg = std::format("Failed to load icon {}! The file doesn't exist!", path.string());
				EPOCH_ASSERT(false, msg);
			}

			*aTexture = Texture2D::Create(path);
		}
	};
}

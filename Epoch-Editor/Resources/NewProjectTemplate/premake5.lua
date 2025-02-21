EpochRootDirectory = os.getenv("EPOCH_DIR")
include (path.join(EpochRootDirectory, "Resources", "LUA", "Epoch.lua"))

workspace "$PROJECT_NAME$"
	startproject "$PROJECT_NAME$"
	
	configurations 
	{ 
		"Debug", 
		"Release",
		"Dist"
	}
	
	filter "configurations:Debug"
		optimize "off"
		symbols "on"
	
	filter "configurations:Release"
		optimize "on"
		symbols "default"
	
	filter "configurations:Dist"
		optimize "full"
		symbols "off"


group "Epoch"

	project "Epoch-ScriptCore"
		location "%{EpochRootDirectory}/Epoch-ScriptCore"
		kind "SharedLib"
		language "C#"
		dotnetframework "4.7.2"
	
		targetdir ("%{EpochRootDirectory}/Resources/Scripts")
		objdir ("%{EpochRootDirectory}/Resources/Scripts/Intermediates")
	
		files
		{
			"%{EpochRootDirectory}/Epoch-ScriptCore/Source/**.cs",
			"%{EpochRootDirectory}/Epoch-ScriptCore/Properties/**.cs"
		}

		linkAppReferences()
		
group ""

	
project "$PROJECT_NAME$"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetname "$PROJECT_NAME$"
	targetdir ("%{prj.location}/Scripts/Binaries")
	objdir ("%{prj.location}/Scripts/Intermediates")

	files 
	{
		"Assets/**.cs", 
	}

	linkAppReferences()

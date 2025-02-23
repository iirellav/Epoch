EpochRootDirectory = os.getenv("EPOCH_DIR")

workspace "DefaultProject"
	startproject "DefaultProject"
	
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
		namespace "Epoch"
	
		targetdir ("%{EpochRootDirectory}/Aeon/Resources/Scripts")
		objdir ("%{EpochRootDirectory}/Aeon/Resources/Scripts/Intermediates")
	
		files
		{
			"%{EpochRootDirectory}/Epoch-ScriptCore/Source/**.cs",
			"%{EpochRootDirectory}/Epoch-ScriptCore/Properties/**.cs"
		}
		
group ""

	
project "DefaultProject"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetname "DefaultProject"
	targetdir ("%{prj.location}/Scripts/Binaries")
	objdir ("%{prj.location}/Scripts/Intermediates")

	files 
	{
		"Assets/**.cs", 
	}

	local monoLibsPath = path.join(EpochRootDirectory, "Aeon", "mono", "lib", "mono", "4.5"):gsub("/", "\\")
    local monoLibsFacadesPath = path.join(monoLibsPath, "Facades"):gsub("/", "\\")

	libdirs { monoLibsPath, monoLibsFacadesPath }
	links { "Epoch-ScriptCore" }

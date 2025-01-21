project "Aeon"
	kind "ConsoleApp"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"resource.h",
		"Aeon.rc"
	}

	includedirs
	{
		"../CommonUtilities/src",
		"src",
		"../Epoch/src",
		"../Epoch/vendor",
		"../Epoch/vendor/GLFW/include",
		"../Epoch/vendor/filewatch/include",
		"../Epoch/vendor/yaml-cpp/include",
		"../Epoch/vendor/NFD-Extended/include",
		"../Epoch/vendor/magic_enum/include"
	}

	links
	{
		"Epoch"
	}

	filter "configurations:Debug"
		postbuildcommands { "{COPYFILE} %[../Epoch/vendor/mono/bin/Debug/mono-2.0-sgen.dll] %[../bin/$(Configuration)-$(LlvmPlatformName)/$(ProjectName)]" }
		postbuildcommands { "{COPYFILE} %[../Epoch/vendor/assimp/bin/Debug/assimp-vc143-mtd.dll] %[../bin/$(Configuration)-$(LlvmPlatformName)/$(ProjectName)]" }

	filter "configurations:Release or configurations:Dist"
		postbuildcommands { "{COPYFILE} %[../Epoch/vendor/mono/bin/Release/mono-2.0-sgen.dll] %[../bin/$(Configuration)-$(LlvmPlatformName)/$(ProjectName)]" }
		postbuildcommands { "{COPYFILE} %[../Epoch/vendor/assimp/bin/Release/assimp-vc143-mt.dll] %[../bin/$(Configuration)-$(LlvmPlatformName)/$(ProjectName)]" }

	filter "configurations:Dist"
		kind "WindowedApp"

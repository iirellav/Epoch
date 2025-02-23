project "Epoch-Runtime"
	kind "ConsoleApp"
	targetname "Runtime"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"resource.h",
		"Epoch-Runtime.rc"
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
		"../Epoch/vendor/magic_enum/include",
		"../Epoch/vendor/tracy/tracy"
	}

	links
	{
		"Epoch"
	}

	filter "configurations:R-Debug"
		postbuildcommands { "{COPYFILE} %[../Epoch/vendor/mono/bin/Debug/mono-2.0-sgen.dll] %[../bin/$(Configuration)-$(LlvmPlatformName)/$(ProjectName)]" }

	filter "configurations:R-Release or configurations:R-Dist"
		postbuildcommands { "{COPYFILE} %[../Epoch/vendor/mono/bin/Release/mono-2.0-sgen.dll] %[../bin/$(Configuration)-$(LlvmPlatformName)/$(ProjectName)]" }

	filter "configurations:R-Dist"
		kind "WindowedApp"

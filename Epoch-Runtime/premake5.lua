project "Epoch-Runtime"
	kind "ConsoleApp"
	targetname "Runtime"
	filter "configurations:R-Release"
		targetname "Runtime_Dev"

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
		--postbuildcommands { "xcopy \"$(EPOCH_DIR)Epoch\\vendor\\mono\\bin\\Debug\\mono-2.0-sgen.dll\" \"$(EPOCH_DIR)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }
		--postbuildcommands { "xcopy \"$(EPOCH_DIR)Epoch\\vendor\\assimp\\bin\\Debug\\assimp-vc143-mtd.dll\" \"$(EPOCH_DIR)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }

	filter "configurations:R-Release or configurations:R-Dist"
		--postbuildcommands { "xcopy \"$(EPOCH_DIR)Epoch\\vendor\\mono\\bin\\Release\\mono-2.0-sgen.dll\" \"$(EPOCH_DIR)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }
		--postbuildcommands { "xcopy \"$(EPOCH_DIR)Epoch\\vendor\\assimp\\bin\\Release\\assimp-vc143-mt.dll\" \"$(EPOCH_DIR)bin\\$(Configuration)-$(LlvmPlatformName)\\$(ProjectName)\" /e /y /i /r" }

	filter "configurations:R-Dist"
		kind "WindowedApp"

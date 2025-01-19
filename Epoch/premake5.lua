project "Epoch"
	kind "StaticLib"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "epch.h"
	pchsource "src/epch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",

		"vendor/FastNoise/**.cpp"
	}

	includedirs
	{
		"../CommonUtilities/src",
		"src",
		"vendor",
		"vendor/GLFW/include",
		"vendor/filewatch/include",
		"vendor/yaml-cpp/include",
		"vendor/NFD-Extended/include",
		"vendor/magic_enum/include",
		"vendor/assimp/include",
		"vendor/mono/include",
		"vendor/msdf-atlas-gen/msdf-atlas-gen",
		"vendor/msdf-atlas-gen/msdfgen",
		"vendor/PhysX/include"
	}

	links
	{
		"CommonUtilities",
		"GLFW",

		"d3d11",
		"dxgi",
		"d3dcompiler",
		"dxguid",

		"PhysX_static_64",
		"PhysXCharacterKinematic_static_64",
		"PhysXCommon_static_64",
		"PhysXExtensions_static_64",
		"PhysXFoundation_static_64",
		"PhysXPvdSDK_static_64",
		
		"ImGui",
		"yaml-cpp",
		"NFD-Extended",
		"mono-2.0-sgen",

		"msdf-atlas-gen",
		"msdfgen",
		"freetype"
	}

	defines "PX_PHYSX_STATIC_LIB"

	filter "files:vendor/FastNoise/**.cpp"
		flags { "NoPCH" }

	filter "configurations:Debug"
		links
		{
			"assimp-vc143-mtd"
		}
		libdirs
		{
			"vendor/mono/lib/Debug",
			"vendor/assimp/bin/Debug",
			"vendor/PhysX/lib/Debug"
		}

	filter "configurations:Release"
		links
		{
			"assimp-vc143-mt"
		}
		libdirs
		{
			"vendor/mono/lib/Release",
			"vendor/assimp/bin/Release",
			"vendor/PhysX/lib/Profile"
		}

	filter "configurations:Dist"
		links
		{
			"assimp-vc143-mt"
		}
		libdirs
		{
			"vendor/mono/lib/Release",
			"vendor/assimp/bin/Release",
			"vendor/PhysX/lib/Release"
		}

	-- Runtime configs
	filter "configurations:R-Debug"
		libdirs
		{
			"vendor/mono/lib/Debug",
			"vendor/PhysX/lib/Debug"
		}

	filter "configurations:R-Release"
		libdirs
		{
			"vendor/mono/lib/Release",
			"vendor/PhysX/lib/Profile"
		}

	filter "configurations:R-Dist"
		libdirs
		{
			"vendor/mono/lib/Release",
			"vendor/PhysX/lib/Release"
		}

	filter "configurations:R-Debug or configurations:R-Release or configurations:R-Dist"
		removefiles
		{
			"src/Epoch/Assets/AssimpMeshImporter.cpp"
		}
		defines 
		{
			"_RUNTIME"
		}

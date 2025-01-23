project "Tracy"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"tracy/client/**.h",
		"tracy/client/**.hpp",
		"tracy/client/**.cpp",

		"tracy/common/**.h",
		"tracy/common/**.hpp",
		"tracy/common/**.cpp",

		"tracy/tracy/**.h",
		"tracy/tracy/**.hpp",
		"tracy/tracy/**.cpp",

		"tracy/libbacktrace/alloc.cpp",
		"tracy/libbacktrace/sort.cpp",
		"tracy/libbacktrace/state.cpp",
	}

	includedirs { "tracy/" }

	links
	{
		"DbgHelp"
	}

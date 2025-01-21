workspace "Epoch"
	architecture "x64"
	startproject  "Aeon"

	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	configurations { "Debug", "Release", "Dist", "R-Debug", "R-Release", "R-Dist" }

	outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"

	flags { "MultiProcessorCompile" }

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
		"_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING"
	}

	filter "action:vs*"
        linkoptions { "/ignore:4099" } -- Disable no PDB found warning
        disablewarnings { "4068" } -- Disable "Unknown #pragma mark warning"


	filter "system:windows"
		systemversion "latest"
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
		defines { "PLATFORM_WINDOWS" }
	
	filter "configurations:Debug or configurations:R-Debug"
		defines { "_DEBUG" }
		optimize "off"
		symbols "on"
		
	filter "configurations:Release or configurations:Runtime-Release"
		defines { "_RELEASE", "NDEBUG" }
		optimize "on"
		symbols "default"
		
	filter "configurations:Dist or configurations:R-Dist"
		defines { "_DIST", "NDEBUG" }
		optimize "full"
		symbols "off"

	filter "configurations:R-Debug or configurations:R-Release or configurations:R-Dist"
		defines 
		{
			"_RUNTIME"
		}

group "Core"
include "Epoch"
include "Aeon/Epoch-ScriptCore"
group ""

group "Tools"
include "Aeon"
--include "Basic Raytracer"
--include "SSOHorseRegistry"
group ""

group "Runtime"
include "Epoch-Runtime"
group ""

group "Dependencies"
include "CommonUtilities"
project "GLFW"
	location "Epoch/vendor/GLFW"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Epoch/vendor/GLFW/include/GLFW/glfw3.h",
		"Epoch/vendor/GLFW/include/GLFW/glfw3native.h",
		"Epoch/vendor/GLFW/src/glfw_config.h",
		"Epoch/vendor/GLFW/src/context.c",
		"Epoch/vendor/GLFW/src/init.c",
		"Epoch/vendor/GLFW/src/input.c",
		"Epoch/vendor/GLFW/src/monitor.c",

		"Epoch/vendor/GLFW/src/null_init.c",
		"Epoch/vendor/GLFW/src/null_joystick.c",
		"Epoch/vendor/GLFW/src/null_monitor.c",
		"Epoch/vendor/GLFW/src/null_window.c",

		"Epoch/vendor/GLFW/src/platform.c",
		"Epoch/vendor/GLFW/src/vulkan.c",
		"Epoch/vendor/GLFW/src/window.c",
	}

	filter "system:windows"
		files
		{
			"Epoch/vendor/GLFW/src/win32_init.c",
			"Epoch/vendor/GLFW/src/win32_joystick.c",
			"Epoch/vendor/GLFW/src/win32_module.c",
			"Epoch/vendor/GLFW/src/win32_monitor.c",
			"Epoch/vendor/GLFW/src/win32_time.c",
			"Epoch/vendor/GLFW/src/win32_thread.c",
			"Epoch/vendor/GLFW/src/win32_window.c",
			"Epoch/vendor/GLFW/src/wgl_context.c",
			"Epoch/vendor/GLFW/src/egl_context.c",
			"Epoch/vendor/GLFW/src/osmesa_context.c"
		}

		defines { "_GLFW_WIN32" }

project "ImGui"
	location "Epoch/vendor/imgui"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Epoch/vendor/imgui/**.h",
		"Epoch/vendor/imgui/**.cpp",
		
		"Epoch/vendor/ImGuizmo/**.h",
		"Epoch/vendor/ImGuizmo/**.cpp"
	}

	includedirs
	{
		"Epoch/vendor/imgui",
		"Epoch/vendor/ImGuizmo",
		"Epoch/vendor/GLFW/include",
	}

	links
	{
		"GLFW"
	}

project "yaml-cpp"
	location "Epoch/vendor/yaml-cpp"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Epoch/vendor/yaml-cpp/src/**.h",
		"Epoch/vendor/yaml-cpp/src/**.cpp",

		"Epoch/vendor/yaml-cpp/include/**.h"
	}

	includedirs
	{
		"Epoch/vendor/yaml-cpp/include"
	}

project "NFD-Extended"
	location "Epoch/vendor/NFD-Extended"
	kind "StaticLib"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Epoch/vendor/NFD-Extended/include/nfd.h",
		"Epoch/vendor/NFD-Extended/include/nfd.hpp",

		"Epoch/vendor/NFD-Extended/nfd_win.cpp"
	}

	includedirs
	{
		"Epoch/vendor/NFD-Extended/include"
	}
	
group "Dependencies/msdf"
include "Epoch/vendor/msdf-atlas-gen"
group ""
group ""
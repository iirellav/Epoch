include (path.join("../Resources/LUA/Epoch.lua"))

project "Epoch-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	linkAppReferences(false)

	targetdir ("../Resources/Scripts")
	objdir ("../Resources/Scripts/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

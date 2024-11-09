include (path.join("../Aeon/Resources/LUA/Epoch.lua"))

project "Epoch-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	linkAppReferences(false)

	targetdir ("../Aeon/Resources/Scripts")
	objdir ("../Aeon/Resources/Scripts/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

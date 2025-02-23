local function getAssemblyFiles(directory)
	handle = io.popen("dir " .. directory .. " /B /A-D")

	t = {}
	for f in handle:lines() do
		if path.hasextension(f, ".dll") then
			if string.find(f, "System.") then
				table.insert(t, f)
			end
		end
	end

	handle:close()
	return t
end

function linkAppReferences(linkScriptCore)
	local epochDir = os.getenv("EPOCH_DIR")
	local monoLibsPath
    local monoLibsFacadesPath

	monoLibsPath = path.join(epochDir, "mono", "lib", "mono", "4.5"):gsub("/", "\\")
	monoLibsFacadesPath = path.join(monoLibsPath, "Facades"):gsub("/", "\\")

	libdirs { monoLibsPath, monoLibsFacadesPath }
	if linkScriptCore ~= false then
		links { "Epoch-ScriptCore" }
	end

	for k, v in ipairs(getAssemblyFiles(monoLibsPath)) do
		--print("Adding reference to: " .. v)
		links { v }
	end

	for k, v in ipairs(getAssemblyFiles(monoLibsFacadesPath)) do
        --print("Adding reference to: " .. v)
        links { v }
    end
end
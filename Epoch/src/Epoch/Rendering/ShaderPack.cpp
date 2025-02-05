#include "epch.h"
#include "ShaderPack.h"
#include "Shader.h"
#include "Epoch/Serialization/FileStream.h"

namespace Epoch
{
    ShaderPack::ShaderPack(const std::filesystem::path& aPath) : myPath(aPath)
    {
        FileStreamReader serializer(aPath);
        if (!serializer)
        {
            return;
        }

        serializer.ReadRaw(myFile.header);
        if (memcmp(myFile.header.HEADER, "EPSP", 4) != 0)
        {
            return;
        }

        myLoaded = true;
        for (uint32_t i = 0; i < myFile.header.shaderProgramCount; i++)
        {
            uint32_t key;
            serializer.ReadRaw(key);
            auto& shaderProgramInfo = myFile.indexTable.shaderPrograms[key];
            serializer.ReadArray(shaderProgramInfo.moduleIndices);
        }

        auto sp = serializer.GetStreamPosition();
        serializer.ReadArray(myFile.indexTable.shaderModules, myFile.header.shaderModuleCount);
    }

    bool ShaderPack::Contains(std::string_view aPath) const
    {
        return myFile.indexTable.shaderPrograms.find((uint32_t)Hash::GenerateFNVHash(aPath)) != myFile.indexTable.shaderPrograms.end();
    }

    std::shared_ptr<Shader> ShaderPack::LoadShader(std::string_view aPath)
    {
        uint32_t hash = (uint32_t)Hash::GenerateFNVHash(aPath);
        EPOCH_ASSERT(Contains(aPath), "Shader pack doesn't contain shader '{}'", aPath);

        const auto& shaderProgramInfo = myFile.indexTable.shaderPrograms.at(hash);

        FileStreamReader serializer(myPath);

        std::map<ShaderStage, std::vector<uint8_t>> shaderModules;
        for (uint32_t index : shaderProgramInfo.moduleIndices)
        {
            const auto& info = myFile.indexTable.shaderModules[index];
            auto& moduleData = shaderModules[(ShaderStage)info.stage];

            serializer.SetStreamPosition(info.packedOffset);
            serializer.ReadArray(moduleData, (uint32_t)info.packedSize);
        }

        std::shared_ptr<Shader> shader = Shader::Create();
        shader->CreateShaders(shaderModules);

        shader->myName = std::filesystem::path(aPath).stem().string();
        shader->myFilePath = aPath;

        return shader;
    }

    std::shared_ptr<ShaderPack> ShaderPack::CreateFromLibrary(std::shared_ptr<ShaderLibrary> aShaderLibrary, const std::filesystem::path& aPath)
    {
        std::shared_ptr<ShaderPack> shaderPack = std::make_shared<ShaderPack>(aPath);

        const auto& shaderMap = aShaderLibrary->GetShaders();
        auto& shaderPackFile = shaderPack->myFile;

        shaderPackFile.header.version = 1;
        shaderPackFile.header.shaderProgramCount = (uint32_t)shaderMap.size();
        shaderPackFile.header.shaderModuleCount = 0;

        // Determine number of modules (per shader)
        uint32_t shaderModuleIndex = 0;
        uint32_t shaderModuleIndexArraySize = 0;
        for (const auto& [name, shader] : shaderMap)
        {
            //std::shared_ptr<Shader> vulkanShader = shader.As<VulkanShader>();
            const auto& shaderData = shader->myShaderData;

            shaderPackFile.header.shaderModuleCount += (uint32_t)shaderData.size();
            auto& shaderProgramInfo = shaderPackFile.indexTable.shaderPrograms[shader->GetHash()];

            for (int i = 0; i < (int)shaderData.size(); i++)
            {
                shaderProgramInfo.moduleIndices.emplace_back(shaderModuleIndex++);
            }

            shaderModuleIndexArraySize += sizeof(uint32_t); // size
            shaderModuleIndexArraySize += (uint32_t)shaderData.size() * sizeof(uint32_t); // indices
        }

        uint32_t shaderProgramIndexSize = shaderPackFile.header.shaderProgramCount * (sizeof(std::map<uint32_t, ShaderPackFile::ShaderProgramInfo>::key_type)) + shaderModuleIndexArraySize;

        FileStreamWriter serializer(aPath);

        // Write header
        serializer.WriteRaw<ShaderPackFile::FileHeader>(shaderPackFile.header);

        // ===============
        // Write index
        // ===============
        // Write dummy data for shader programs
        uint64_t shaderProgramIndexPos = serializer.GetStreamPosition();
        serializer.WriteZero(shaderProgramIndexSize);

        // Write dummy data for shader modules
        uint64_t shaderModuleIndexPos = serializer.GetStreamPosition();
        serializer.WriteZero(shaderPackFile.header.shaderModuleCount * sizeof(ShaderPackFile::ShaderModuleInfo));
        for (const auto& [name, shader] : shaderMap)
        {
            // Serialize shader data
            const auto& shaderData = shader->myShaderData;
            for (const auto& [stage, data] : shaderData)
            {
                auto& indexShaderModule = shaderPackFile.indexTable.shaderModules.emplace_back();
                indexShaderModule.packedOffset = serializer.GetStreamPosition();
                indexShaderModule.packedSize = data.size();
                indexShaderModule.stage = (uint8_t)stage;

                serializer.WriteArray(data, false);
            }
        }

        // Write program index
        serializer.SetStreamPosition(shaderProgramIndexPos);
        uint64_t begin = shaderProgramIndexPos;
        for (const auto& [name, programInfo] : shaderPackFile.indexTable.shaderPrograms)
        {
            serializer.WriteRaw(name);
            serializer.WriteArray(programInfo.moduleIndices);
        }
        uint64_t end = serializer.GetStreamPosition();

        // Write module index
        serializer.SetStreamPosition(shaderModuleIndexPos);
        serializer.WriteArray(shaderPackFile.indexTable.shaderModules, false);

        CONSOLE_LOG_INFO("Serialized {} shader programs and {} shader modules into shader pack", shaderPackFile.indexTable.shaderPrograms.size(), shaderPackFile.indexTable.shaderModules.size());

        return shaderPack;
    }
}

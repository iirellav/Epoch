#include "epch.h"
#include "MeshRuntimeSerializer.h"
#include "Epoch/Rendering/Mesh.h"
#include "Epoch/Assets/AssetManager.h"
#include "Epoch/Rendering/VertexBuffer.h"
#include "Epoch/Rendering/IndexBuffer.h"
#include "MeshFile.h"

namespace Epoch
{
    bool MeshRuntimeSerializer::SerializeToAssetPack(AssetHandle aHandle, FileStreamWriter& aStream, AssetSerializationInfo& outInfo)
    {
        uint64_t streamOffset = aStream.GetStreamPosition();
        outInfo.offset = streamOffset;

        std::shared_ptr<Mesh> meshAsset = AssetManager::GetAsset<Mesh>(aHandle);

        MeshFile file;

        // Write header
        aStream.WriteRaw<MeshFile::FileHeader>(file.header);
        // Leave space for Metadata
        uint64_t metadataAbsolutePosition = aStream.GetStreamPosition();
        aStream.WriteZero(sizeof(MeshFile::Metadata));

        file.data.boundingBox = meshAsset->GetBoundingBox();

        // Write nodes
        file.data.nodeArrayOffset = aStream.GetStreamPosition() - streamOffset;
        aStream.WriteArray(meshAsset->myNodes);
        file.data.nodeArraySize = (aStream.GetStreamPosition() - streamOffset) - file.data.nodeArrayOffset;

        // Write submeshes
        file.data.submeshArrayOffset = aStream.GetStreamPosition() - streamOffset;
        aStream.WriteArray(meshAsset->mySubmeshes);
        file.data.submeshArraySize = (aStream.GetStreamPosition() - streamOffset) - file.data.submeshArrayOffset;

        // Write Vertex Buffer
        file.data.vertexBufferOffset = aStream.GetStreamPosition() - streamOffset;
        aStream.WriteArray(meshAsset->myVertices);
        file.data.vertexBufferSize = (aStream.GetStreamPosition() - streamOffset) - file.data.vertexBufferOffset;

        // Write Index Buffer
        file.data.indexBufferOffset = aStream.GetStreamPosition() - streamOffset;
        aStream.WriteArray(meshAsset->myIndices);
        file.data.indexBufferSize = (aStream.GetStreamPosition() - streamOffset) - file.data.indexBufferOffset;

        // Write Metadata
        uint64_t endOfStream = aStream.GetStreamPosition();
        aStream.SetStreamPosition(metadataAbsolutePosition);
        aStream.WriteRaw<MeshFile::Metadata>(file.data);
        aStream.SetStreamPosition(endOfStream);

        outInfo.size = aStream.GetStreamPosition() - outInfo.offset;
        return outInfo.size > 0;
    }

    std::shared_ptr<Asset> MeshRuntimeSerializer::DeserializeFromAssetPack(FileStreamReader& aStream, const AssetPackFile::AssetInfo& aAssetInfo)
    {
        aStream.SetStreamPosition(aAssetInfo.packedOffset);
        uint64_t streamOffset = aStream.GetStreamPosition();

        MeshFile file;
        aStream.ReadRaw<MeshFile::FileHeader>(file.header);
        bool validHeader = memcmp(file.header.HEADER, "EPMF", 4) == 0;
        EPOCH_ASSERT(validHeader);
        if (!validHeader)
        {
            return nullptr;
        }

        std::shared_ptr<Mesh> meshAsset = std::make_shared<Mesh>();

        aStream.ReadRaw<MeshFile::Metadata>(file.data);

        const auto& metadata = file.data;

        meshAsset->myBoundingBox = metadata.boundingBox;

        aStream.SetStreamPosition(metadata.nodeArrayOffset + streamOffset);
        aStream.ReadArray(meshAsset->myNodes);
        aStream.SetStreamPosition(metadata.submeshArrayOffset + streamOffset);
        aStream.ReadArray(meshAsset->mySubmeshes);

        aStream.SetStreamPosition(metadata.vertexBufferOffset + streamOffset);
        aStream.ReadArray(meshAsset->myVertices);

        aStream.SetStreamPosition(metadata.indexBufferOffset + streamOffset);
        aStream.ReadArray(meshAsset->myIndices);

        if (!meshAsset->myVertices.empty())
        {
            meshAsset->myVertexBuffer = VertexBuffer::Create(meshAsset->myVertices.data(), (uint32_t)meshAsset->myVertices.size(), sizeof(Vertex));
        }

        if (!meshAsset->myIndices.empty())
        {
            meshAsset->myIndexBuffer = IndexBuffer::Create(meshAsset->myIndices.data(), (uint32_t)meshAsset->myIndices.size());
        }

        return meshAsset;
    }
}

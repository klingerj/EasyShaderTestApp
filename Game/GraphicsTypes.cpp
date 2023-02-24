#include "GraphicsTypes.h"

#include <string.h>

using namespace Tk;

void CreateDefaultGeometry(Graphics::GraphicsCommandStream* graphicsCommandStream)
{
    // Default Quad
    {
        Graphics::ResourceDesc desc;
        desc.resourceType = Graphics::ResourceType::eBuffer1D;
        desc.debugLabel = "Default Quad Vtx Attr Buffer";

        // Positions
        desc.dims = v3ui(sizeof(defaultQuad.m_points), 0, 0);
        desc.bufferUsage = Graphics::BufferUsage::eVertex;
        defaultQuad.m_positionBuffer.gpuBufferHandle = Graphics::CreateResource(desc);

        desc.bufferUsage = Graphics::BufferUsage::eStaging;
        Graphics::ResourceHandle stagingBufferHandle_Pos = Graphics::CreateResource(desc);
        void* stagingBufferMemPtr_Pos = Graphics::MapResource(stagingBufferHandle_Pos);

        // UVs
        desc.dims = v3ui(sizeof(defaultQuad.m_uvs), 0, 0);
        desc.bufferUsage = Graphics::BufferUsage::eVertex;
        defaultQuad.m_uvBuffer.gpuBufferHandle = Graphics::CreateResource(desc);

        desc.bufferUsage = Graphics::BufferUsage::eStaging;
        Graphics::ResourceHandle stagingBufferHandle_UV = Graphics::CreateResource(desc);
        void* stagingBufferMemPtr_UV = Graphics::MapResource(stagingBufferHandle_UV);

        // Normals
        desc.dims = v3ui(sizeof(defaultQuad.m_normals), 0, 0);
        desc.bufferUsage = Graphics::BufferUsage::eVertex;
        defaultQuad.m_normalBuffer.gpuBufferHandle = Graphics::CreateResource(desc);

        desc.bufferUsage = Graphics::BufferUsage::eStaging;
        Graphics::ResourceHandle stagingBufferHandle_Norm = Graphics::CreateResource(desc);
        void* stagingBufferMemPtr_Norm = Graphics::MapResource(stagingBufferHandle_Norm);

        // Indices
        desc.dims = v3ui(sizeof(defaultQuad.m_indices), 0, 0);
        desc.bufferUsage = Graphics::BufferUsage::eIndex;
        defaultQuad.m_indexBuffer.gpuBufferHandle = Graphics::CreateResource(desc);

        desc.bufferUsage = Graphics::BufferUsage::eStaging;
        Graphics::ResourceHandle stagingBufferHandle_Idx = Graphics::CreateResource(desc);
        void* stagingBufferMemPtr_Idx = Graphics::MapResource(stagingBufferHandle_Idx);

        // Descriptor
        CreateDefaultGeometryVertexBufferDescriptor(defaultQuad);

        // Memcpy into staging buffers
        memcpy(stagingBufferMemPtr_Pos, defaultQuad.m_points, sizeof(defaultQuad.m_points));
        memcpy(stagingBufferMemPtr_UV, defaultQuad.m_uvs, sizeof(defaultQuad.m_uvs));
        memcpy(stagingBufferMemPtr_Norm, defaultQuad.m_normals, sizeof(defaultQuad.m_normals));
        memcpy(stagingBufferMemPtr_Idx, defaultQuad.m_indices, sizeof(defaultQuad.m_indices));

        // Do GPU buffer copies
        Graphics::GraphicsCommand* command = &graphicsCommandStream->m_graphicsCommands[graphicsCommandStream->m_numCommands];
        command->m_commandType = Graphics::GraphicsCommand::eMemTransfer;
        command->debugLabel = "Update Default Quad Vtx Pos Buf";
        command->m_sizeInBytes = sizeof(defaultQuad.m_points);
        command->m_dstBufferHandle = defaultQuad.m_positionBuffer.gpuBufferHandle;
        command->m_srcBufferHandle = stagingBufferHandle_Pos;
        ++graphicsCommandStream->m_numCommands;
        ++command;

        command->m_commandType = Graphics::GraphicsCommand::eMemTransfer;
        command->debugLabel = "Update Default Quad Vtx UV Buf";
        command->m_sizeInBytes = sizeof(defaultQuad.m_uvs);
        command->m_dstBufferHandle = defaultQuad.m_uvBuffer.gpuBufferHandle;
        command->m_srcBufferHandle = stagingBufferHandle_UV;
        ++graphicsCommandStream->m_numCommands;
        ++command;

        command->m_commandType = Graphics::GraphicsCommand::eMemTransfer;
        command->debugLabel = "Update Default Quad Vtx Norm Buf";
        command->m_sizeInBytes = sizeof(defaultQuad.m_normals);
        command->m_dstBufferHandle = defaultQuad.m_normalBuffer.gpuBufferHandle;
        command->m_srcBufferHandle = stagingBufferHandle_Norm;
        ++graphicsCommandStream->m_numCommands;
        ++command;

        command->m_commandType = Graphics::GraphicsCommand::eMemTransfer;
        command->debugLabel = "Update Default Quad Vtx Idx Buf";
        command->m_sizeInBytes = sizeof(defaultQuad.m_indices);
        command->m_dstBufferHandle = defaultQuad.m_indexBuffer.gpuBufferHandle;
        command->m_srcBufferHandle = stagingBufferHandle_Idx;
        ++graphicsCommandStream->m_numCommands;
        ++command;

        Graphics::SubmitCmdsImmediate(graphicsCommandStream);
        graphicsCommandStream->m_numCommands = 0; // reset the cmd counter for the stream

        // Unmap + destroy resources
        Graphics::UnmapResource(stagingBufferHandle_Pos);
        Graphics::UnmapResource(stagingBufferHandle_UV);
        Graphics::UnmapResource(stagingBufferHandle_Norm);
        Graphics::UnmapResource(stagingBufferHandle_Idx);

        Graphics::DestroyResource(stagingBufferHandle_Pos);
        Graphics::DestroyResource(stagingBufferHandle_UV);
        Graphics::DestroyResource(stagingBufferHandle_Norm);
        Graphics::DestroyResource(stagingBufferHandle_Idx);
    }
}

void DestroyDefaultGeometry()
{
    // Default quad
    Graphics::DestroyResource(defaultQuad.m_positionBuffer.gpuBufferHandle);
    defaultQuad.m_positionBuffer.gpuBufferHandle = Graphics::DefaultResHandle_Invalid;
    Graphics::DestroyResource(defaultQuad.m_uvBuffer.gpuBufferHandle);
    defaultQuad.m_uvBuffer.gpuBufferHandle = Graphics::DefaultResHandle_Invalid;
    Graphics::DestroyResource(defaultQuad.m_normalBuffer.gpuBufferHandle);
    defaultQuad.m_normalBuffer.gpuBufferHandle = Graphics::DefaultResHandle_Invalid;
    Graphics::DestroyResource(defaultQuad.m_indexBuffer.gpuBufferHandle);
    defaultQuad.m_indexBuffer.gpuBufferHandle = Graphics::DefaultResHandle_Invalid;

    //DestroyDefaultGeometryVertexBufferDescriptor(defaultQuad);
}

DefaultGeometry<DEFAULT_QUAD_NUM_VERTICES, DEFAULT_QUAD_NUM_INDICES> defaultQuad = {
    // buffer handles
    { Graphics::DefaultResHandle_Invalid },
    { Graphics::DefaultResHandle_Invalid },
    { Graphics::DefaultResHandle_Invalid },
    { Graphics::DefaultResHandle_Invalid },
    // descriptor
    { Graphics::DefaultDescHandle_Invalid },
    // positions
    v4f(-1.0f, -1.0f, 0.0f, 1.0f),
    v4f(1.0f, -1.0f, 0.0f, 1.0f),
    v4f(-1.0f, 1.0f, 0.0f, 1.0f),
    v4f(1.0f, 1.0f, 0.0f, 1.0f),
    //uvs
    v2f(0.0f, 0.0f),
    v2f(0.0f, 1.0f),
    v2f(0.0f, 1.0f),
    v2f(1.0f, 1.0f),
    // normals
    v4f(0.0f, 0.0f, 1.0f, 0.0f),
    v4f(0.0f, 0.0f, 1.0f, 0.0f),
    v4f(0.0f, 0.0f, 1.0f, 0.0f),
    v4f(0.0f, 0.0f, 1.0f, 0.0f),
    // indices
    0, 1, 2, 2, 1, 3
};

void CreateAnimatedPoly(TransientPrim* prim)
{
    Graphics::ResourceDesc desc;
    desc.resourceType = Graphics::ResourceType::eBuffer1D;

    prim->numVertices = 150;

    desc.dims = v3ui(prim->numVertices * sizeof(v4f), 0, 0);
    desc.bufferUsage = Graphics::BufferUsage::eTransientVertex;
    prim->vertexBufferHandle = Graphics::CreateResource(desc);
    desc.bufferUsage = Graphics::BufferUsage::eTransientIndex;
    desc.dims = v3ui((prim->numVertices - 1) * 3 * sizeof(uint32), 0, 0);
    prim->indexBufferHandle = Graphics::CreateResource(desc);

    // Descriptor - vertex buffer
    prim->descriptor = Graphics::CreateDescriptor(Graphics::DESCLAYOUT_ID_POSONLY_VBS);

    Graphics::DescriptorSetDataHandles descDataHandles[MAX_DESCRIPTOR_SETS_PER_SHADER] = {};
    descDataHandles[0].InitInvalid();
    descDataHandles[0].handles[0] = prim->vertexBufferHandle;
    descDataHandles[1].InitInvalid();
    descDataHandles[2].InitInvalid();

    Graphics::WriteDescriptor(Graphics::DESCLAYOUT_ID_POSONLY_VBS, prim->descriptor, &descDataHandles[0]);
}

void DestroyAnimatedPoly(TransientPrim* prim)
{
    Graphics::DestroyResource(prim->indexBufferHandle);
    prim->indexBufferHandle = Graphics::DefaultResHandle_Invalid;
    Graphics::DestroyResource(prim->vertexBufferHandle);
    prim->vertexBufferHandle = Graphics::DefaultResHandle_Invalid;

    Graphics::DestroyDescriptor(prim->descriptor);
    prim->descriptor = Graphics::DefaultDescHandle_Invalid;
}

void UpdateAnimatedPoly(TransientPrim* prim)
{
    // Map
    void* indexBuf = Graphics::MapResource(prim->indexBufferHandle);
    void* vertexBuf = Graphics::MapResource(prim->vertexBufferHandle);

    // Update
    const uint32 numIndices = ((prim->numVertices - 1) * 3);
    for (uint32 idx = 0; idx < numIndices; idx += 3)
    {
        ((uint32*)indexBuf)[idx + 0] = 0;
        ((uint32*)indexBuf)[idx + 1] = idx / 3 + 1;
        ((uint32*)indexBuf)[idx + 2] = idx < numIndices - 3 ? idx / 3 + 2 : 1;
    }

    static uint32 frameCtr = 0;
    ++frameCtr;
    for (uint32 vtx = 0; vtx < prim->numVertices; ++vtx)
    {
        if (vtx == 0)
        {
            ((v4f*)vertexBuf)[vtx] = v4f(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            const float f = ((float)(vtx - 1) / (prim->numVertices - 1));
            const float amt = f * (3.14159f * 2.0f);
            const float scale = 2.0f * (cosf((float)frameCtr * 0.2f * f) * 0.5f + 0.5f);
            ((v4f*)vertexBuf)[vtx] = v4f(cosf(amt) * scale, sinf(amt) * scale, 0.0f, 1.0f);
        }
    }

    // Unmap
    Graphics::UnmapResource(prim->indexBufferHandle);
    Graphics::UnmapResource(prim->vertexBufferHandle);
}

void DrawAnimatedPoly(TransientPrim* prim, Graphics::DescriptorHandle globalData, uint32 shaderID, uint32 blendState, uint32 depthState, Graphics::GraphicsCommandStream* graphicsCommandStream)
{
    Graphics::GraphicsCommand* command = &graphicsCommandStream->m_graphicsCommands[graphicsCommandStream->m_numCommands];

    command->m_commandType = Graphics::GraphicsCommand::eDrawCall;
    command->debugLabel = "Draw animated poly";
    command->m_numIndices = (prim->numVertices - 1) * 3;
    command->m_numInstances = 1;
    command->m_vertOffset = 0;
    command->m_indexOffset = 0;
    command->m_indexBufferHandle = prim->indexBufferHandle;
    command->m_shader = shaderID;
    command->m_blendState = blendState;
    command->m_depthState = depthState;

    for (uint32 i = 0; i < MAX_DESCRIPTOR_SETS_PER_SHADER; ++i)
    {
        command->m_descriptors[i] = Graphics::DefaultDescHandle_Invalid;
    }
    command->m_descriptors[0] = globalData;
    command->m_descriptors[1] = prim->descriptor;
    ++graphicsCommandStream->m_numCommands;
}

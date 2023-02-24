#pragma once

#include "Graphics/Common/GraphicsCommon.h"

struct GameRenderPass
{
    uint32 numColorRTs;
    Tk::Graphics::ResourceHandle colorRTs[MAX_MULTIPLE_RENDERTARGETS];
    Tk::Graphics::ResourceHandle depthRT;
    uint32 renderWidth;
    uint32 renderHeight;
    const char* debugLabel;

    void Init()
    {
        numColorRTs = 0;
        renderWidth = 0;
        renderWidth = 0;
        debugLabel = NULL;
        depthRT = Tk::Graphics::DefaultResHandle_Invalid;
        for (uint32 i = 0; i < ARRAYCOUNT(colorRTs); ++i)
        {
            colorRTs[i] = Tk::Graphics::DefaultResHandle_Invalid;
        }
    }
};

void StartRenderPass(GameRenderPass* renderPass, Tk::Graphics::GraphicsCommandStream* graphicsCommandStream);
void EndRenderPass(GameRenderPass* renderPass, Tk::Graphics::GraphicsCommandStream* graphicsCommandStream);
void DrawMeshDataCommand(Tk::Graphics::GraphicsCommandStream* graphicsCommandStream, uint32 numIndices,
    uint32 numInstances, Tk::Graphics::ResourceHandle indexBufferHandle,
    uint32 shaderID, uint32 blendState, uint32 depthState,
    Tk::Graphics::DescriptorHandle* descriptors, const char* debugLabel);

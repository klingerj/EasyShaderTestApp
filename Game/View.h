#pragma once

#include "Platform/PlatformGameAPI.h"
#include "Math/VectorTypes.h"
#include "GraphicsTypes.h"

struct View
{
    alignas(CACHE_LINE) m4f m_viewMatrix;
    alignas(CACHE_LINE) m4f m_projMatrix;
    alignas(CACHE_LINE) m4f m_viewProjMatrix;
};
void Init(View* view);
void Update(View* view, Tk::Graphics::DescriptorSetDataHandles* descDataHandles);

struct GameRenderPass;
void RecordRenderPassCommands(View* view, Scene* scene, GameRenderPass* renderPass,
    Tk::Graphics::GraphicsCommandStream* graphicsCommandStream, uint32 shaderID, uint32 blendState, uint32 depthState,
    Tk::Graphics::DescriptorHandle* descriptors);

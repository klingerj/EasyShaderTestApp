#include "RenderPass.h"

#include <string.h>

using namespace Tk;

void DrawMeshDataCommand(Graphics::GraphicsCommandStream* graphicsCommandStream, uint32 numIndices,
    uint32 numInstances, Graphics::ResourceHandle indexBufferHandle,
    uint32 shaderID, uint32 blendState, uint32 depthState,
    Graphics::DescriptorHandle* descriptors, const char* debugLabel)
{
    Graphics::GraphicsCommand* command = &graphicsCommandStream->m_graphicsCommands[graphicsCommandStream->m_numCommands];

    command->m_commandType = Graphics::GraphicsCommand::eDrawCall;
    command->debugLabel = debugLabel;

    command->m_numIndices = numIndices;
    command->m_numInstances = numInstances;
    command->m_vertOffset = 0;
    command->m_indexOffset = 0;
    command->m_shader = shaderID;
    command->m_blendState = blendState;
    command->m_depthState = depthState;
    command->m_indexBufferHandle = indexBufferHandle;
    memcpy(command->m_descriptors, descriptors, sizeof(Graphics::DescriptorHandle) * MAX_DESCRIPTOR_SETS_PER_SHADER);
    ++graphicsCommandStream->m_numCommands;
}

void StartRenderPass(GameRenderPass* renderPass, Graphics::GraphicsCommandStream* graphicsCommandStream)
{
    Graphics::GraphicsCommand* command = &graphicsCommandStream->m_graphicsCommands[graphicsCommandStream->m_numCommands];

    command->m_commandType = Graphics::GraphicsCommand::eRenderPassBegin;
    command->debugLabel = renderPass->debugLabel;
    
    uint32 numColorRTs = renderPass->numColorRTs;
    command->m_numColorRTs = numColorRTs;
    if (numColorRTs)
        memcpy(command->m_colorRTs, renderPass->colorRTs, sizeof(Graphics::ResourceHandle) * numColorRTs);
    command->m_depthRT = renderPass->depthRT;
    command->m_renderWidth = renderPass->renderWidth;
    command->m_renderHeight = renderPass->renderHeight;
    ++graphicsCommandStream->m_numCommands;
    ++command;

    // Set scissor state
    command->m_commandType = Graphics::GraphicsCommand::eSetScissor;
    command->debugLabel = "Set render pass scissor state";
    command->m_scissorOffsetX = 0;
    command->m_scissorOffsetY = 0;
    command->m_scissorWidth = renderPass->renderWidth;
    command->m_scissorHeight = renderPass->renderHeight;
    ++graphicsCommandStream->m_numCommands;
}

void EndRenderPass(GameRenderPass* renderPass, Graphics::GraphicsCommandStream* graphicsCommandStream)
{
    Graphics::GraphicsCommand* command = &graphicsCommandStream->m_graphicsCommands[graphicsCommandStream->m_numCommands];

    command->m_commandType = Graphics::GraphicsCommand::eRenderPassEnd;
    command->debugLabel = renderPass->debugLabel;
    ++graphicsCommandStream->m_numCommands;
}

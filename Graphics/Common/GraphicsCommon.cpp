#include "GraphicsCommon.h"
#include "GPUTimestamps.h"
#include "Utility/Logging.h"

#ifdef VULKAN
#include "Graphics/Vulkan/Vulkan.h"
#include "Graphics/Vulkan/VulkanTypes.h"
#endif

namespace Tk
{
namespace Graphics
{

uint32 MultiBufferedStatusFromBufferUsage[] = 
{
    0u,
    0u,
    1u,
    1u,
    0u,
    1u,
};
static_assert(ARRAYCOUNT(MultiBufferedStatusFromBufferUsage) == BufferUsage::eMax); // Don't forget to add one here if enum is added to

void CreateContext(const Tk::Platform::WindowHandles* windowHandles, uint32 windowWidth, uint32 windowHeight)
{
    int result = 0;

    #ifdef VULKAN
    result = InitVulkan(windowHandles, windowWidth, windowHeight);
    #endif

    if (result)
        Core::Utility::LogMsg("Platform", "Failed to init graphics backend!", Core::Utility::LogSeverity::eCritical);
}

void RecreateContext(const Tk::Platform::WindowHandles* windowHandles, uint32 windowWidth, uint32 windowHeight)
{
    #ifdef VULKAN
    DestroyVulkan();
    InitVulkan(windowHandles, windowWidth, windowHeight);
    #endif
}

void WindowResize()
{
    #ifdef VULKAN
    VulkanDestroyAllPSOPerms();
    VulkanDestroySwapChain();
    VulkanCreateSwapChain();
    #endif
}

void WindowMinimized()
{
    #ifdef VULKAN
    g_vulkanContextResources.isSwapChainValid = false;
    #endif
}

void DestroyContext()
{
    #ifdef VULKAN
    DestroyVulkan();
    #endif
}

void DestroyAllPSOPerms()
{
    #ifdef VULKAN
    VulkanDestroyAllPSOPerms();
    #endif
}

bool AcquireFrame()
{
    #ifdef VULKAN
    if (g_vulkanContextResources.isSwapChainValid)
        return VulkanAcquireFrame();
    
    return false;
    #else
    return false;
    #endif
}

void ProcessGraphicsCommandStream(const GraphicsCommandStream* graphicsCommandStream, bool immediateSubmit)
{
    TINKER_ASSERT(graphicsCommandStream->m_numCommands <= graphicsCommandStream->m_maxCommands);

    const bool multithreadedCmdRecording = false;

    if (multithreadedCmdRecording)
    {
        // TODO: implement :)
    }
    else
    {
        uint32 currentShaderID = SHADER_ID_MAX;
        uint32 currentBlendState = BlendState::eMax;
        uint32 currentDepthState = DepthState::eMax;
        DescriptorHandle currDescriptors[MAX_DESCRIPTOR_SETS_PER_SHADER];
        for (uint32 i = 0; i < MAX_DESCRIPTOR_SETS_PER_SHADER; ++i)
        {
            currDescriptors[i] = Graphics::DefaultDescHandle_Invalid;
        }

        for (uint32 i = 0; i < graphicsCommandStream->m_numCommands; ++i)
        {
            TINKER_ASSERT(graphicsCommandStream->m_graphicsCommands[i].m_commandType < GraphicsCommand::eMax);

            const GraphicsCommand& currentCmd = graphicsCommandStream->m_graphicsCommands[i];

            switch (currentCmd.m_commandType)
            {
                case GraphicsCommand::eDrawCall:
                {
                    const bool psoChange =
                        currentShaderID != currentCmd.m_shader ||
                        (currentBlendState != currentCmd.m_blendState) ||
                        (currentDepthState != currentCmd.m_depthState);
                    
                    currentShaderID = currentCmd.m_shader;
                    currentBlendState = currentCmd.m_blendState;
                    currentDepthState = currentCmd.m_depthState;

                    if (psoChange)
                    {
                        RecordCommandBindShader(currentShaderID, currentBlendState, currentDepthState, immediateSubmit);
                    }

                    for (uint32 uiDesc = 0; uiDesc < MAX_DESCRIPTOR_SETS_PER_SHADER; ++uiDesc)
                    {
                        if (currDescriptors[uiDesc] != currentCmd.m_descriptors[uiDesc])
                        {
                            RecordCommandBindDescriptor(currentShaderID, currentCmd.m_descriptors[uiDesc], uiDesc, immediateSubmit);
                        }
                    }

                    RecordCommandDrawCall(currentCmd.m_indexBufferHandle, currentCmd.m_numIndices,
                        currentCmd.m_numInstances, currentCmd.m_vertOffset, currentCmd.m_indexOffset,
                        currentCmd.debugLabel, immediateSubmit);
                    break;
                }

                case GraphicsCommand::eMemTransfer:
                {
                    RecordCommandMemoryTransfer(currentCmd.m_sizeInBytes, currentCmd.m_srcBufferHandle, currentCmd.m_dstBufferHandle,
                        currentCmd.debugLabel, immediateSubmit);

                    break;
                }

                case GraphicsCommand::ePushConstant:
                {
                    RecordCommandPushConstant(&currentCmd.m_pushConstantData[0], ARRAYCOUNT(currentCmd.m_pushConstantData) * sizeof(uint8), currentCmd.m_shaderForLayout);

                    break;
                }

                case GraphicsCommand::eSetScissor:
                {
                    RecordCommandSetScissor(currentCmd.m_scissorOffsetX, currentCmd.m_scissorOffsetY, currentCmd.m_scissorWidth, currentCmd.m_scissorHeight);

                    break;
                }

                case GraphicsCommand::eRenderPassBegin:
                {
                    RecordCommandRenderPassBegin(currentCmd.m_numColorRTs, &currentCmd.m_colorRTs[0], currentCmd.m_depthRT,
                        currentCmd.m_renderWidth, currentCmd.m_renderHeight, currentCmd.debugLabel, immediateSubmit);

                    break;
                }

                case GraphicsCommand::eRenderPassEnd:
                {
                    RecordCommandRenderPassEnd(immediateSubmit);

                    break;
                }

                case GraphicsCommand::eLayoutTransition:
                {
                    RecordCommandTransitionLayout(currentCmd.m_imageHandle,
                        currentCmd.m_startLayout, currentCmd.m_endLayout,
                        currentCmd.debugLabel, immediateSubmit);

                    break;
                }

                case GraphicsCommand::eClearImage:
                {
                    RecordCommandClearImage(currentCmd.m_imageHandle,
                        currentCmd.m_clearValue, currentCmd.debugLabel, immediateSubmit);

                    break;
                }

                case GraphicsCommand::eGPUTimestamp:
                {
                    TINKER_ASSERT(GPUTimestamps::GetMostRecentRecordedTimestampCount() <= GPU_TIMESTAMP_NUM_MAX);
                    if (currentCmd.m_timestampStartFrame)
                    {
                        void* cpuCopyBuffer = GPUTimestamps::GetRawCPUSideTimestampBuffer();
                        const uint32 numTimestampsRecorded = GPUTimestamps::GetMostRecentRecordedTimestampCount();
                        ResolveMostRecentAvailableTimestamps(cpuCopyBuffer, numTimestampsRecorded, immediateSubmit);
                        GPUTimestamps::ProcessTimestamps();
                    }

                    RecordCommandGPUTimestamp(GPUTimestamps::GetMostRecentRecordedTimestampCount(), immediateSubmit);
                    GPUTimestamps::RecordName(currentCmd.m_timestampNameStr);

                    break;
                }

                default:
                {
                    // Invalid command type
                    TINKER_ASSERT(0);
                    break;
                }
            }
        }
    }
}

void BeginFrameRecording()
{
    #ifdef VULKAN
    BeginVulkanCommandRecording();
    #endif
}

void EndFrameRecording()
{
    #ifdef VULKAN
    EndVulkanCommandRecording();
    #endif
}

void SubmitFrameToGPU()
{
    #ifdef VULKAN
    Graphics::VulkanSubmitFrame();
    #endif
}

SUBMIT_CMDS_IMMEDIATE(SubmitCmdsImmediate)
{
    #ifdef VULKAN
    BeginVulkanCommandRecordingImmediate();
    ProcessGraphicsCommandStream(graphicsCommandStream, true);
    EndVulkanCommandRecordingImmediate();
    #endif
}

CREATE_RESOURCE(CreateResource)
{
    #ifdef VULKAN
    return VulkanCreateResource(resDesc);
    #else
    return Graphics::DefaultResHandle_Invalid;
    #endif
}

DESTROY_RESOURCE(DestroyResource)
{
    #ifdef VULKAN
    VulkanDestroyResource(handle);
    #endif
}

MAP_RESOURCE(MapResource)
{
    #ifdef VULKAN
    return VulkanMapResource(handle);
    #else
    return NULL;
    #endif
}

UNMAP_RESOURCE(UnmapResource)
{
    #ifdef VULKAN
    VulkanUnmapResource(handle);
    #endif
}

CREATE_GRAPHICS_PIPELINE(CreateGraphicsPipeline)
{
    #ifdef VULKAN
    return VulkanCreateGraphicsPipeline(vertexShaderCode, numVertexShaderBytes, fragmentShaderCode, numFragmentShaderBytes,
        shaderID, viewportWidth, viewportHeight, numColorRTs, colorRTFormats, depthFormat, descriptorHandles, numDescriptorHandles);
    #else
    return false;
    #endif
}

CREATE_DESCRIPTOR(CreateDescriptor)
{
    #ifdef VULKAN
    return VulkanCreateDescriptor(descLayoutID);
    #else
    return DefaultDescHandle_Invalid;
    #endif
}

DESTROY_DESCRIPTOR(DestroyDescriptor)
{
    #ifdef VULKAN
    VulkanDestroyDescriptor(handle);
    #endif
}

DESTROY_ALL_DESCRIPTORS(DestroyAllDescriptors)
{
    #ifdef VULKAN
    VulkanDestroyAllDescriptors();
    #endif
}

WRITE_DESCRIPTOR(WriteDescriptor)
{
    #ifdef VULKAN
    VulkanWriteDescriptor(descLayoutID, descSetHandle, descSetDataHandles);
    #endif
}

CREATE_DESCRIPTOR_LAYOUT(CreateDescriptorLayout)
{
    #ifdef VULKAN
    return VulkanCreateDescriptorLayout(descLayoutID, descLayout);
    #else
    return false;
    #endif

}

DESTROY_GRAPHICS_PIPELINE(DestroyGraphicsPipeline)
{
    #ifdef VULKAN
    DestroyPSOPerms(shaderID);
    #endif
}

}
}

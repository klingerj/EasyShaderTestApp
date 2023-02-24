#include "Graphics/Common/GraphicsCommon.h"
#include "Graphics/Vulkan/Vulkan.h"
#include "Graphics/Vulkan/VulkanTypes.h"
#include "Utility/Logging.h"

namespace Tk
{
namespace Graphics
{

bool VulkanAcquireFrame()
{
    const VulkanVirtualFrameSyncData& virtualFrameSyncData = g_vulkanContextResources.virtualFrameSyncData[g_vulkanContextResources.currentVirtualFrame];

    VkResult result = vkWaitForFences(g_vulkanContextResources.device, 1, &virtualFrameSyncData.Fence, VK_FALSE, (uint64)-1);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Waiting for virtual frame fence took too long!", Core::Utility::LogSeverity::eInfo);
        return false;
    }
    vkResetFences(g_vulkanContextResources.device, 1, &virtualFrameSyncData.Fence);

    uint32 currentSwapChainImageIndex = TINKER_INVALID_HANDLE;

    result = vkAcquireNextImageKHR(g_vulkanContextResources.device,
        g_vulkanContextResources.swapChain,
        (uint64)-1,
        virtualFrameSyncData.ImageAvailableSema,
        VK_NULL_HANDLE,
        &currentSwapChainImageIndex);

    // Error checking
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        Core::Utility::LogMsg("Platform", "Failed to acquire next swap chain image!", Core::Utility::LogSeverity::eInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            Core::Utility::LogMsg("Platform", "out of date swap chain!", Core::Utility::LogSeverity::eInfo);
        }
        else
        {
            Core::Utility::LogMsg("Platform", "Not recreating swap chain!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0); // we will probably crash
        }
        return false; // Don't present on this frame
    }

    g_vulkanContextResources.currentSwapChainImage = currentSwapChainImageIndex;
    return true;
}

void VulkanSubmitFrame()
{
    const VulkanVirtualFrameSyncData& virtualFrameSyncData = g_vulkanContextResources.virtualFrameSyncData[g_vulkanContextResources.currentVirtualFrame];

    // Submit
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[1] = { virtualFrameSyncData.ImageAvailableSema };
    VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_vulkanContextResources.commandBuffers[g_vulkanContextResources.currentVirtualFrame];

    VkSemaphore signalSemaphores[1] = { virtualFrameSyncData.GPUWorkCompleteSema };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult result = vkQueueSubmit(g_vulkanContextResources.graphicsQueue, 1, &submitInfo, virtualFrameSyncData.Fence);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to submit command buffer to queue!", Core::Utility::LogSeverity::eCritical);
    }

    // Present
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &g_vulkanContextResources.swapChain;
    presentInfo.pImageIndices = &g_vulkanContextResources.currentSwapChainImage;

    result = vkQueuePresentKHR(g_vulkanContextResources.graphicsQueue, &presentInfo);

    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to present swap chain!", Core::Utility::LogSeverity::eInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            Core::Utility::LogMsg("Platform", "Out of date / suboptimal swap chain (probably minimized window)!", Core::Utility::LogSeverity::eInfo);
            
        }
        else
        {
            Core::Utility::LogMsg("Platform", "Not recreating swap chain!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0); // we will probably crash
        }
        return; // don't present on this frame
    }

    g_vulkanContextResources.currentVirtualFrame = (g_vulkanContextResources.currentVirtualFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    ++g_vulkanContextResources.frameCounter;
}

void* VulkanMapResource(ResourceHandle handle)
{
    VulkanMemResourceChain* resourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(handle.m_hRes);
    const ResourceDesc& desc = resourceChain->resDesc;
    VulkanMemResource* resource = &resourceChain->resourceChain[IsBufferUsageMultiBuffered(desc.bufferUsage) ?  g_vulkanContextResources.currentVirtualFrame : 0];

    // Note: Right now, all host visible memory is allocated into the same single device memory block, which I just leave persistently mapped. So this just has to return the mapped ptr + offset.
    if (1)
    {
        return (void*)((uint8*)resource->GpuMemAlloc.mappedMemPtr + resource->GpuMemAlloc.allocOffset);
    }
    else
    {
        void* newMappedMem;
        VkResult result = vkMapMemory(g_vulkanContextResources.device, resource->GpuMemAlloc.allocMem, 0, VK_WHOLE_SIZE, 0, &newMappedMem);

        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to map gpu memory!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
            return nullptr;
        }

        return newMappedMem;
    }
}

void VulkanUnmapResource(ResourceHandle handle)
{
    VulkanMemResourceChain* resourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(handle.m_hRes);
    const ResourceDesc& desc = resourceChain->resDesc;
    VulkanMemResource* resource = &resourceChain->resourceChain[IsBufferUsageMultiBuffered(desc.bufferUsage) ? g_vulkanContextResources.currentVirtualFrame : 0];

    // Flush right before unmapping
    VkMappedMemoryRange memoryRange = {};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.memory = resource->GpuMemAlloc.allocMem;
    memoryRange.offset = resource->GpuMemAlloc.allocOffset;
    memoryRange.size = resource->GpuMemAlloc.allocSize;

    VkResult result = vkFlushMappedMemoryRanges(g_vulkanContextResources.device, 1, &memoryRange);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to flush mapped gpu memory!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    if (0)
    {
        vkUnmapMemory(g_vulkanContextResources.device, resource->GpuMemAlloc.allocMem);
    }
}

void VulkanWriteDescriptor(uint32 descriptorLayoutID, DescriptorHandle descSetHandle, const DescriptorSetDataHandles* descSetDataHandles)
{
    DescriptorLayout* descLayout = &g_vulkanContextResources.descLayouts[descriptorLayoutID].bindings;

    for (uint32 uiImage = 0; uiImage < MAX_FRAMES_IN_FLIGHT; ++uiImage)
    {
        VkDescriptorSet* descriptorSet =
            &g_vulkanContextResources.vulkanDescriptorResourcePool.PtrFromHandle(descSetHandle.m_hDesc)->resourceChain[uiImage].descriptorSet;

        // Descriptor layout
        VkWriteDescriptorSet descSetWrites[MAX_BINDINGS_PER_SET] = {};
        uint32 descriptorCount = 0;

        // Desc set info data
        VkDescriptorBufferInfo descBufferInfo[MAX_BINDINGS_PER_SET] = {};
        VkDescriptorImageInfo descImageInfo[MAX_BINDINGS_PER_SET] = {};

        for (uint32 uiDesc = 0; uiDesc < MAX_BINDINGS_PER_SET; ++uiDesc)
        {
            descSetWrites[uiDesc].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

            uint32 type = descLayout->params[uiDesc].type;
            if (type != DescriptorType::eMax)
            {
                VulkanMemResourceChain* resChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(descSetDataHandles->handles[uiDesc].m_hRes);
                uint32 resIndex = 0;

                switch (type)
                {
                    case DescriptorType::eBuffer:
                    case DescriptorType::eSSBO:
                    {
                        resIndex = IsBufferUsageMultiBuffered(resChain->resDesc.bufferUsage) ? uiImage : 0u;

                        VkBuffer* buffer = &resChain->resourceChain[resIndex].buffer;

                        descBufferInfo[descriptorCount].buffer = *buffer;
                        descBufferInfo[descriptorCount].offset = 0;
                        descBufferInfo[descriptorCount].range = VK_WHOLE_SIZE;

                        descSetWrites[descriptorCount].dstSet = *descriptorSet;
                        descSetWrites[descriptorCount].dstBinding = descriptorCount;
                        descSetWrites[descriptorCount].dstArrayElement = 0;
                        descSetWrites[descriptorCount].descriptorType = GetVkDescriptorType(type);
                        descSetWrites[descriptorCount].descriptorCount = 1;
                        descSetWrites[descriptorCount].pBufferInfo = &descBufferInfo[descriptorCount];
                        break;
                    }

                    case DescriptorType::eSampledImage:
                    {
                        VkImageView* imageView = &resChain->resourceChain[resIndex].imageView; // Should be index 0 for images

                        descImageInfo[descriptorCount].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descImageInfo[descriptorCount].imageView = *imageView;
                        descImageInfo[descriptorCount].sampler = g_vulkanContextResources.linearSampler;

                        descSetWrites[descriptorCount].dstSet = *descriptorSet;
                        descSetWrites[descriptorCount].dstBinding = descriptorCount;
                        descSetWrites[descriptorCount].dstArrayElement = 0;
                        descSetWrites[descriptorCount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descSetWrites[descriptorCount].descriptorCount = 1;
                        descSetWrites[descriptorCount].pImageInfo = &descImageInfo[descriptorCount];
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
                ++descriptorCount;
            }
        }

        if (descriptorCount > 0)
        {
            vkUpdateDescriptorSets(g_vulkanContextResources.device, descriptorCount, descSetWrites, 0, nullptr);
        }
    }
}

void BeginVulkanCommandRecording()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkResult result = vkBeginCommandBuffer(g_vulkanContextResources.commandBuffers[g_vulkanContextResources.currentVirtualFrame], &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to begin Vulkan command buffer!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
}

void EndVulkanCommandRecording()
{
    if (g_vulkanContextResources.currentSwapChainImage == TINKER_INVALID_HANDLE || g_vulkanContextResources.currentVirtualFrame == TINKER_INVALID_HANDLE)
    {
        TINKER_ASSERT(0);
    }

    VkResult result = vkEndCommandBuffer(g_vulkanContextResources.commandBuffers[g_vulkanContextResources.currentVirtualFrame]);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to end Vulkan command buffer!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
}

void BeginVulkanCommandRecordingImmediate()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult result = vkBeginCommandBuffer(g_vulkanContextResources.commandBuffer_Immediate, &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to begin Vulkan command buffer (immediate)!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
}

void EndVulkanCommandRecordingImmediate()
{
    VkResult result = vkEndCommandBuffer(g_vulkanContextResources.commandBuffer_Immediate);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to end Vulkan command buffer (immediate)!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_vulkanContextResources.commandBuffer_Immediate;

    result = vkQueueSubmit(g_vulkanContextResources.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to submit command buffer to queue!", Core::Utility::LogSeverity::eCritical);
    }
    vkQueueWaitIdle(g_vulkanContextResources.graphicsQueue);
}

// TODO: remove mystery bool param
VkCommandBuffer ChooseAppropriateCommandBuffer(bool immediateSubmit)
{
    VkCommandBuffer commandBuffer = g_vulkanContextResources.commandBuffer_Immediate;

    if (!immediateSubmit)
    {
        TINKER_ASSERT(g_vulkanContextResources.currentSwapChainImage != TINKER_INVALID_HANDLE && g_vulkanContextResources.currentVirtualFrame != TINKER_INVALID_HANDLE);
        commandBuffer = g_vulkanContextResources.commandBuffers[g_vulkanContextResources.currentVirtualFrame];
    }

    return commandBuffer;
}

void RecordCommandPushConstant(const uint8* data, uint32 sizeInBytes, uint32 shaderID)
{
    TINKER_ASSERT(data && sizeInBytes);

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(false);
    const VkPipelineLayout& pipelineLayout = g_vulkanContextResources.psoPermutations.pipelineLayout[shaderID];
    TINKER_ASSERT(pipelineLayout != VK_NULL_HANDLE);

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeInBytes, data);
}

void RecordCommandSetScissor(int32 offsetX, int32 offsetY, uint32 width, uint32 height)
{
    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(false);
    VkRect2D scissor = {};
    scissor.offset = { offsetX, offsetY };
    scissor.extent = { width, height };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void RecordCommandDrawCall(ResourceHandle indexBufferHandle, uint32 numIndices, uint32 numInstances,
    uint32 vertOffset, uint32 indexOffset, const char* debugLabel, bool immediateSubmit)
{
    TINKER_ASSERT(indexBufferHandle != DefaultResHandle_Invalid);

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    // Index buffer
    VulkanMemResourceChain* indexBufferResourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(indexBufferHandle.m_hRes);
    const ResourceDesc& desc = indexBufferResourceChain->resDesc;
    VkBuffer& indexBuffer = indexBufferResourceChain->resourceChain[IsBufferUsageMultiBuffered(desc.bufferUsage) ? g_vulkanContextResources.currentVirtualFrame : 0].buffer;
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, numIndices, numInstances, indexOffset, vertOffset, 0);
}

void RecordCommandBindShader(uint32 shaderID, uint32 blendState, uint32 depthState, bool immediateSubmit)
{
    const VkPipeline& pipeline = g_vulkanContextResources.psoPermutations.graphicsPipeline[shaderID][blendState][depthState];
    TINKER_ASSERT(pipeline != VK_NULL_HANDLE);

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void RecordCommandBindDescriptor(uint32 shaderID, const DescriptorHandle descSetHandle, uint32 descSetIndex, bool immediateSubmit)
{
    TINKER_ASSERT(descSetHandle != DefaultDescHandle_Invalid);
    const VkPipelineLayout& pipelineLayout = g_vulkanContextResources.psoPermutations.pipelineLayout[shaderID];
    TINKER_ASSERT(pipelineLayout != VK_NULL_HANDLE);

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    VkDescriptorSet* descSet =
        &g_vulkanContextResources.vulkanDescriptorResourcePool.PtrFromHandle(descSetHandle.m_hDesc)->resourceChain[g_vulkanContextResources.currentVirtualFrame].descriptorSet;

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descSetIndex, 1, descSet, 0, nullptr);

    /*for (uint32 uiDesc = 0; uiDesc < MAX_DESCRIPTOR_SETS_PER_SHADER; ++uiDesc)
    {
        DescriptorHandle descHandle = descSetHandles[uiDesc];
        if (descHandle != DefaultDescHandle_Invalid)
        {
            VkDescriptorSet* descSet =
                &g_vulkanContextResources.vulkanDescriptorResourcePool.PtrFromHandle(descHandle.m_hDesc)->resourceChain[g_vulkanContextResources.currentVirtualFrame].descriptorSet;

            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, uiDesc, 1, descSet, 0, nullptr);
        }
    }*/
}

void RecordCommandMemoryTransfer(uint32 sizeInBytes, ResourceHandle srcBufferHandle, ResourceHandle dstBufferHandle,
    const char* debugLabel, bool immediateSubmit)
{
    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    VulkanMemResourceChain* dstResourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(dstBufferHandle.m_hRes);
    VulkanMemResourceChain* srcResourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(srcBufferHandle.m_hRes);

    switch (dstResourceChain->resDesc.resourceType)
    {
        case ResourceType::eBuffer1D:
        {
            VkBufferCopy bufferCopy = {};
            bufferCopy.srcOffset = 0; // TODO: make this stuff a function param?
            bufferCopy.size = sizeInBytes;
            bufferCopy.dstOffset = 0;

            uint32 srcIndex = IsBufferUsageMultiBuffered(srcResourceChain->resDesc.bufferUsage) ? g_vulkanContextResources.currentVirtualFrame : 0u;
            uint32 dstIndex = IsBufferUsageMultiBuffered(dstResourceChain->resDesc.bufferUsage) ? g_vulkanContextResources.currentVirtualFrame : 0u;
            VkBuffer srcBuffer = srcResourceChain->resourceChain[srcIndex].buffer;
            VkBuffer dstBuffer = dstResourceChain->resourceChain[dstIndex].buffer;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

            break;
        }

        case ResourceType::eImage2D:
        {
            uint32 bytesPerPixel = 0;
            switch (dstResourceChain->resDesc.imageFormat)
            {
                case ImageFormat::BGRA8_SRGB:
                case ImageFormat::RGBA8_SRGB:
                {
                    bytesPerPixel = 32 / 8;
                    break;
                }

                default:
                {
                    Core::Utility::LogMsg("Platform", "Unsupported image copy dst format!", Core::Utility::LogSeverity::eCritical);
                    TINKER_ASSERT(0);
                    return;
                }
            }

            {
                // TODO: currently no images are duplicated per frame in flight
                VkBuffer& srcBuffer = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(srcBufferHandle.m_hRes)->resourceChain[0].buffer;
                VkImage& dstImage = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(dstBufferHandle.m_hRes)->resourceChain[0].image;

                // TODO: make some of these into function params
                VkBufferImageCopy region = {};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = dstResourceChain->resDesc.arrayEles;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { dstResourceChain->resDesc.dims.x, dstResourceChain->resDesc.dims.y, dstResourceChain->resDesc.dims.z };

                vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            }
            break;
        }
    }
}

void RecordCommandRenderPassBegin(uint32 numColorRTs, const ResourceHandle* colorRTs, ResourceHandle depthRT, uint32 renderWidth, uint32 renderHeight,
    const char* debugLabel, bool immediateSubmit)
{
    const bool HasDepth = depthRT.m_hRes != TINKER_INVALID_HANDLE;
    const uint32 numAttachments = numColorRTs + (HasDepth ? 1u : 0u);

    if (HasDepth)
        TINKER_ASSERT(numAttachments <= VULKAN_MAX_RENDERTARGETS_WITH_DEPTH);
    else
        TINKER_ASSERT(numAttachments <= VULKAN_MAX_RENDERTARGETS);

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);
    
    VkRenderingAttachmentInfo colorAttachments[VULKAN_MAX_RENDERTARGETS] = {};
    for (uint32 i = 0; i < Min(numColorRTs, VULKAN_MAX_RENDERTARGETS); ++i)
    {
        VkRenderingAttachmentInfo& colorAttachment = colorAttachments[i];
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

        if (colorRTs[i] == IMAGE_HANDLE_SWAP_CHAIN)
        {
            colorAttachment.imageView = g_vulkanContextResources.swapChainImageViews[g_vulkanContextResources.currentSwapChainImage];
        }
        else
        {
            VulkanMemResource* resource = &g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(colorRTs[i].m_hRes)->resourceChain[0]; // Currently, all image resources are not duplicated per frame in flight
            colorAttachment.imageView = resource->imageView;
        }
    }

    VkRenderingAttachmentInfo depthAttachment = {};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.color = { DEPTH_MAX, 0 };
    if (HasDepth)
    {
        VulkanMemResource* resource = &g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(depthRT.m_hRes)->resourceChain[0]; // Currently, all image resources are not duplicated per frame in flight
        depthAttachment.imageView = resource->imageView;
    }

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { 0, 0, renderWidth, renderHeight };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = numColorRTs;
    renderingInfo.pColorAttachments = numColorRTs ? colorAttachments : nullptr;
    renderingInfo.pDepthAttachment = HasDepth ? &depthAttachment : nullptr;
    renderingInfo.pStencilAttachment = nullptr;

    DbgStartMarker(commandBuffer, debugLabel);

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}

void RecordCommandRenderPassEnd(bool immediateSubmit)
{
    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);
    vkCmdEndRendering(commandBuffer);
    DbgEndMarker(commandBuffer);
}

void RecordCommandTransitionLayout(ResourceHandle imageHandle,
    uint32 startLayout, uint32 endLayout, const char* debugLabel, bool immediateSubmit)
{
    if (startLayout == endLayout)
    {
        // Useless transition / error transition, don't record it
        TINKER_ASSERT(0);
        return;
    }

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.oldLayout = GetVkImageLayout(startLayout);
    barrier.newLayout = GetVkImageLayout(endLayout);

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    switch (startLayout)
    {
        case ImageLayout::eUndefined:
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = 0;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        }

        case ImageLayout::eShaderRead:
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        }

        case ImageLayout::eTransferDst:
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        }

        case ImageLayout::eDepthOptimal:
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        }

        case ImageLayout::eRenderOptimal:
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        }

        default:
        {
            Core::Utility::LogMsg("Platform", "Invalid dst image resource layout specified for layout transition!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
            return;
        }
    }

    switch (endLayout)
    {
        case ImageLayout::eUndefined:
        {
            TINKER_ASSERT(0);
            // Can't transition to undefined according to Vulkan spec
            return;
        }

        case ImageLayout::eShaderRead:
        {
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        }

        case ImageLayout::eTransferDst:
        {
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        }

        case ImageLayout::eDepthOptimal:
        {
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        }

        case ImageLayout::eRenderOptimal:
        {
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        }

        case ImageLayout::ePresent:
        {
            barrier.dstAccessMask = 0;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        }

        default:
        {
            Core::Utility::LogMsg("Platform", "Invalid src image resource layout specified for layout transition!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
            return;
        }
    }

    VkImage image = VK_NULL_HANDLE;
    uint32 numArrayEles = 0;
    if (imageHandle == IMAGE_HANDLE_SWAP_CHAIN)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image = g_vulkanContextResources.swapChainImages[g_vulkanContextResources.currentSwapChainImage];
        numArrayEles = 1;
    }
    else
    {
        VulkanMemResourceChain* memResourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(imageHandle.m_hRes);
        VulkanMemResource* memResource = &memResourceChain->resourceChain[0]; // Currently, all image resources are not duplicated per frame in flight
        image = memResource->image;
        numArrayEles = memResourceChain->resDesc.arrayEles;

        switch (memResourceChain->resDesc.imageFormat)
        {
            case ImageFormat::BGRA8_SRGB:
            case ImageFormat::RGBA8_SRGB:
            {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                break;
            }

            case ImageFormat::Depth_32F:
            {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            }

            default:
            {
                Core::Utility::LogMsg("Platform", "Invalid image format for layout transition command!", Core::Utility::LogSeverity::eCritical);
                TINKER_ASSERT(0);
                return;
            }
        }
    }

    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = numArrayEles;

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void RecordCommandClearImage(ResourceHandle imageHandle,
    const v4f& clearValue, const char* debugLabel, bool immediateSubmit)
{
    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    VulkanMemResourceChain* memResourceChain = nullptr;
    VulkanMemResource* memResource = nullptr;
    uint32 imageFormat = ImageFormat::Invalid;

    if (imageHandle == IMAGE_HANDLE_SWAP_CHAIN)
    {
        //imageFormat = ImageFormat::TheSwapChainFormat;
        TINKER_ASSERT(0); // don't clear the swap chain for now
    }
    else
    {
        memResourceChain = g_vulkanContextResources.vulkanMemResourcePool.PtrFromHandle(imageHandle.m_hRes);
        memResource = &memResourceChain->resourceChain[0]; // Currently, all image resources are not duplicated per frame in flight
        imageFormat = memResourceChain->resDesc.imageFormat;
    }

    VkClearColorValue clearColor = {};
    VkClearDepthStencilValue clearDepth = {};

    VkImageSubresourceRange range = {};
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    switch (imageFormat)
    {
        case ImageFormat::BGRA8_SRGB:
        case ImageFormat::RGBA8_SRGB:
        {
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            for (uint32 i = 0; i < 4; ++i)
            {
                clearColor.uint32[i] = *(uint32*)&clearValue[i];
            }

            vkCmdClearColorImage(commandBuffer,
                memResource->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

            break;
        }

        case ImageFormat::Depth_32F:
        {
            range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            clearDepth.depth = clearValue.x;
            clearDepth.stencil = *(uint32*)&clearValue.y;

            vkCmdClearDepthStencilImage(commandBuffer,
                memResource->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepth, 1, &range);

            break;
        }

        case ImageFormat::TheSwapChainFormat:
        {
            // TODO: similar to regular color, but need to grab swap chain image handle from vulkan resources rather than following mem resource ptr
        }
        
        default:
        {
            Core::Utility::LogMsg("Platform", "Invalid image format for clear command!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
            return;
        }
    }
}

void RecordCommandGPUTimestamp(uint32 gpuTimestampID, bool immediateSubmit)
{
    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);

    uint32 currQueryOffset = g_vulkanContextResources.currentVirtualFrame * GPU_TIMESTAMP_NUM_MAX + gpuTimestampID;
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, g_vulkanContextResources.queryPoolTimestamp, currQueryOffset);
}

void ResolveMostRecentAvailableTimestamps(void* gpuTimestampCPUSideBuffer, uint32 numTimestampsInQuery, bool immediateSubmit)
{
    uint32 currQueryOffset = g_vulkanContextResources.currentVirtualFrame * GPU_TIMESTAMP_NUM_MAX;

    // Need every virtual frame to happen once before reading real values from older frames
    if (g_vulkanContextResources.frameCounter > MAX_FRAMES_IN_FLIGHT - 1)
    {
        VkResult result = vkGetQueryPoolResults(g_vulkanContextResources.device, g_vulkanContextResources.queryPoolTimestamp, currQueryOffset, numTimestampsInQuery, numTimestampsInQuery * sizeof(uint64), gpuTimestampCPUSideBuffer, sizeof(uint64), VK_QUERY_RESULT_64_BIT);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Graphics", "Failed to get query pool results!", Core::Utility::LogSeverity::eCritical);
        }
    }

    VkCommandBuffer commandBuffer = ChooseAppropriateCommandBuffer(immediateSubmit);
    vkCmdResetQueryPool(commandBuffer, g_vulkanContextResources.queryPoolTimestamp, currQueryOffset, GPU_TIMESTAMP_NUM_MAX);
}

}
}

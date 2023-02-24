#pragma once

#include "CoreDefines.h"
#include "Math/VectorTypes.h"
#include "Platform/PlatformGameAPI.h"
#include "Graphics/Common/GraphicsCommon.h"

namespace Tk
{
namespace Graphics
{

typedef struct vulkan_vertex_position
{
    v4f position;
} VulkanVertexPosition;

typedef struct vulkan_vertex_uv
{
    v2f uv;
} VulkanVertexUV;

typedef struct vulkan_vertex_normal
{
    v3f normal;
} VulkanVertexNormal;

// Init/destroy - called one time
int InitVulkan(const Tk::Platform::WindowHandles* platformWindowHandles, uint32 width, uint32 height);
void DestroyVulkan();

void VulkanCreateSwapChain();
void VulkanDestroySwapChain();

// Frame command recording
bool VulkanAcquireFrame();
void VulkanSubmitFrame();

void BeginVulkanCommandRecording();
void EndVulkanCommandRecording();
void BeginVulkanCommandRecordingImmediate();
void EndVulkanCommandRecordingImmediate();

// Graphics API - resource create/destroy functions
ResourceHandle VulkanCreateResource(const ResourceDesc& resDesc);
void VulkanDestroyResource(ResourceHandle handle);

bool VulkanCreateGraphicsPipeline(void* vertexShaderCode, uint32 numVertexShaderBytes,
    void* fragmentShaderCode, uint32 numFragmentShaderBytes,
    uint32 shaderID, uint32 viewportWidth, uint32 viewportHeight,
    uint32 numColorRTs, const uint32* colorRTFormats, uint32 depthFormat,
    uint32* descriptorLayoutHandles, uint32 numDescriptorLayoutHandles);
void DestroyPSOPerms(uint32 shaderID);
void VulkanDestroyAllPSOPerms();

DescriptorHandle VulkanCreateDescriptor(uint32 descriptorLayoutID);
bool VulkanCreateDescriptorLayout(uint32 descriptorLayoutID, const DescriptorLayout* descriptorLayout);
void VulkanDestroyDescriptor(DescriptorHandle handle);
void VulkanDestroyAllDescriptors();
void DestroyAllDescLayouts();
void VulkanWriteDescriptor(uint32 descriptorLayoutID, DescriptorHandle descSetHandle, const DescriptorSetDataHandles* descSetDataHandles);

// Memory mapping - probably just for staging buffers
void* VulkanMapResource(ResourceHandle handle);
void VulkanUnmapResource(ResourceHandle handle);

}
}

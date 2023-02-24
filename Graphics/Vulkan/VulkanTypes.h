#pragma once

#include "CoreDefines.h"
#include "Allocators.h"
#include "Graphics/Common/GraphicsCommon.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <vulkan/vulkan.h>

#define ENABLE_VULKAN_DEBUG_LABELS // enables marking up vulkan objects/commands with debug labels

#define VULKAN_RESOURCE_POOL_MAX 512

#define VULKAN_NUM_SUPPORTED_DESCRIPTOR_TYPES 3
#define VULKAN_DESCRIPTOR_POOL_MAX_UNIFORM_BUFFERS 64
#define VULKAN_DESCRIPTOR_POOL_MAX_SAMPLED_IMAGES 64
#define VULKAN_DESCRIPTOR_POOL_MAX_STORAGE_BUFFERS 64

#define VULKAN_MAX_RENDERTARGETS MAX_MULTIPLE_RENDERTARGETS
#define VULKAN_MAX_RENDERTARGETS_WITH_DEPTH VULKAN_MAX_RENDERTARGETS + 1 // +1 for depth

namespace Tk
{
namespace Graphics
{

typedef struct vulkan_mem_alloc
{
    VkDeviceMemory allocMem;
    VkDeviceSize allocSize;
    VkDeviceSize allocOffset;
    void* mappedMemPtr;
} VulkanMemAlloc;

typedef struct vulkan_mem_allocator
{
    VkDeviceMemory m_GPUMemory;
    uint64 m_AllocSizeLimit;
    uint64 m_LastAllocOffset;
    uint32 m_NumAllocs;
    VkMemoryPropertyFlags m_MemFlags;
    void* m_MappedMemPtr;
    VkDeviceSize m_AllocGranularity;

    void Init(uint32 TotalAllocSize, uint32 MemoryTypeIndex, VkMemoryPropertyFlagBits MemPropertyFlags, VkDeviceSize AllocGranularity);
    void Destroy();
    VulkanMemAlloc Alloc(VkMemoryRequirements allocReqs);

} VulkanMemoryAllocator;

typedef struct vulkan_mem_resource
{
    VulkanMemAlloc GpuMemAlloc;

    union
    {
        VkBuffer buffer;

        // Group images with an image view
        struct
        {
            VkImage image;
            VkImageView imageView;
        };
    };
} VulkanMemResource;

typedef struct vulkan_descriptor_resource
{
    VkDescriptorSet descriptorSet;
} VulkanDescriptorResource;

// Chains of resources for multiple swap chain images
typedef struct
{
    VulkanMemResource resourceChain[MAX_FRAMES_IN_FLIGHT];
    ResourceDesc resDesc;
} VulkanMemResourceChain;

typedef struct
{
    VulkanDescriptorResource resourceChain[MAX_FRAMES_IN_FLIGHT];
} VulkanDescriptorChain;

typedef struct
{
    VkDescriptorSetLayout layout;
    DescriptorLayout bindings;
} VulkanDescriptorLayout;

typedef struct
{
    VkFence Fence;
    VkSemaphore GPUWorkCompleteSema;
    VkSemaphore ImageAvailableSema;
} VulkanVirtualFrameSyncData;

struct VulkanContextResources
{
    bool isInitted = false;
    bool isSwapChainValid = false;
    uint32 frameCounter = 0;
    
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = NULL;
    PFN_vkCmdBeginDebugUtilsLabelEXT  pfnCmdBeginDebugUtilsLabelEXT  = NULL;
    PFN_vkCmdEndDebugUtilsLabelEXT    pfnCmdEndDebugUtilsLabelEXT    = NULL;
    PFN_vkCmdInsertDebugUtilsLabelEXT pfnCmdInsertDebugUtilsLabelEXT = NULL;
    PFN_vkSetDebugUtilsObjectNameEXT  pfnSetDebugUtilsObjectNameEXT  = NULL;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32 graphicsQueueIndex = TINKER_INVALID_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentationQueue = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkExtent2D swapChainExtent = { 0, 0 };
    VkFormat swapChainFormat = VK_FORMAT_UNDEFINED;
    VkImage* swapChainImages = nullptr;
    VkImageView* swapChainImageViews = nullptr;
    VkFramebuffer* swapChainFramebuffers = nullptr;
    uint32 numSwapChainImages = 0;

    uint32 currentSwapChainImage = TINKER_INVALID_HANDLE;
    uint32 currentVirtualFrame = 0;
    uint32 windowWidth = 0;
    uint32 windowHeight = 0;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkSampler linearSampler = VK_NULL_HANDLE;
    Tk::Core::PoolAllocator<VulkanMemResourceChain> vulkanMemResourcePool;
    Tk::Core::PoolAllocator<VulkanDescriptorChain> vulkanDescriptorResourcePool;
    VulkanVirtualFrameSyncData virtualFrameSyncData[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer* commandBuffers = nullptr;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer_Immediate = VK_NULL_HANDLE;

    enum
    {
        eMaxShaders      = SHADER_ID_MAX,
        eMaxBlendStates  = BlendState::eMax,
        eMaxDepthStates  = DepthState::eMax,
        eMaxDescLayouts  = DESCLAYOUT_ID_MAX,
    };
    struct PSOPerms
    {
        VkPipeline       graphicsPipeline[eMaxShaders][eMaxBlendStates][eMaxDepthStates];
        VkPipelineLayout pipelineLayout[eMaxShaders];
    } psoPermutations;
    VulkanDescriptorLayout descLayouts[eMaxDescLayouts];

    Tk::Core::LinearAllocator DataAllocator;

    enum
    {
        eVulkanMemoryAllocatorDeviceLocalBuffers = 0u,
        eVulkanMemoryAllocatorDeviceLocalImages,
        eVulkanMemoryAllocatorHostVisibleBuffers,
        eVulkanMemoryAllocatorMax,
    };
    VulkanMemoryAllocator GPUMemAllocators[eVulkanMemoryAllocatorMax];

    VkQueryPool queryPoolTimestamp = VK_NULL_HANDLE;
    float timestampPeriod = 0;
};
extern VulkanContextResources g_vulkanContextResources;

void DbgSetImageObjectName(uint64 handle, const char* name);
void DbgSetBufferObjectName(uint64 handle, const char* name);
void DbgStartMarker(VkCommandBuffer commandBuffer, const char* debugLabel);
void DbgEndMarker(VkCommandBuffer commandBuffer);

uint32 ChooseMemoryTypeBits(uint32 requiredMemoryTypeBits, VkMemoryPropertyFlags memPropertyFlags);

VkResult CreateBuffer(VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkBuffer* outBuffer);
VkResult CreateImage(VkImageCreateFlags flags, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32 mipLevels, uint32 arrayLayers, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, VkImage* outImage);

void InitVulkanDataTypesPerEnum();
const VkPipelineColorBlendAttachmentState& GetVkBlendState(uint32 gameBlendState);

typedef struct
{
    VkPipelineDepthStencilStateCreateInfo depthState;
    VkCullModeFlags cullMode;
} DepthCullState;
const DepthCullState& GetVkDepthCullState(uint32 gameDepthCullState);

const VkImageLayout& GetVkImageLayout(uint32 gameImageLayout);
const VkFormat& GetVkImageFormat(uint32 gameImageFormat);
const VkDescriptorType& GetVkDescriptorType(uint32 gameDescriptorType);
VkBufferUsageFlags GetVkBufferUsageFlags(uint32 bufferUsage);
VkMemoryPropertyFlags GetVkMemoryPropertyFlags(uint32 memUsage);

}
}

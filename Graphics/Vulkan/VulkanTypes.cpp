#include "VulkanTypes.h"
#include "Utility/Logging.h"

#include <cstring>

namespace Tk
{
namespace Graphics
{

VulkanContextResources g_vulkanContextResources = {};

void VulkanMemoryAllocator::Init(uint32 TotalAllocSize, uint32 MemoryTypeIndex, VkMemoryPropertyFlagBits MemPropertyFlags, VkDeviceSize AllocGranularity)
{
    m_GPUMemory = VK_NULL_HANDLE;
    m_AllocSizeLimit = TotalAllocSize;
    m_LastAllocOffset = 0;
    m_NumAllocs = 0;
    m_MemFlags = MemPropertyFlags;
    m_AllocGranularity = AllocGranularity;

    VkMemoryAllocateInfo memAllocInfo = {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = m_AllocSizeLimit;
    memAllocInfo.memoryTypeIndex = MemoryTypeIndex;
    VkResult result = vkAllocateMemory(g_vulkanContextResources.device, &memAllocInfo, nullptr, &m_GPUMemory);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "GPU mem allocator failed to allocate gpu memory!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
}

void VulkanMemoryAllocator::Destroy()
{
    vkFreeMemory(g_vulkanContextResources.device, m_GPUMemory, nullptr);
    m_GPUMemory = VK_NULL_HANDLE;
}

VulkanMemAlloc VulkanMemoryAllocator::Alloc(VkMemoryRequirements allocReqs)
{
    uint64 allocSize = RoundValueToPow2(allocReqs.size, m_AllocGranularity);

    uint64 alignment = Max(m_AllocGranularity, allocReqs.alignment);
    TINKER_ASSERT(ISPOW2(alignment));
    uint64 nextAllocOffset = RoundValueToPow2(m_LastAllocOffset, alignment); // ensure alloc offset is aligned
    uint64 nextAllocEnd = nextAllocOffset + allocSize;

    if (nextAllocEnd > m_AllocSizeLimit)
    {
        TINKER_ASSERT(0);
        Core::Utility::LogMsg("Platform", "Vulkan gpu mem allocator ran out of memory!", Core::Utility::LogSeverity::eInfo);
    }

    m_LastAllocOffset = nextAllocEnd;
    ++m_NumAllocs;

    VulkanMemAlloc alloc;
    alloc.allocMem = m_GPUMemory;
    alloc.allocOffset = nextAllocOffset;
    alloc.allocSize = allocSize;
    alloc.mappedMemPtr = m_MappedMemPtr; // may be nullptr
    return alloc;
}

uint32 ChooseMemoryTypeBits(uint32 requiredMemoryTypeBits, VkMemoryPropertyFlags memPropertyFlags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_vulkanContextResources.physicalDevice, &memProperties);

    uint32 memTypeIndex = 0xffffffff;
    // TODO: make this smarter
    // For now, pick the first memory heap that remotely matches
    for (uint32 uiMemType = 0; uiMemType < memProperties.memoryTypeCount; ++uiMemType)
    {
        if (((1 << uiMemType) & requiredMemoryTypeBits) &&
            (memProperties.memoryTypes[uiMemType].propertyFlags & memPropertyFlags) == memPropertyFlags)
        {
            memTypeIndex = uiMemType;
            break;
        }
    }
    return memTypeIndex;
}

VkResult CreateBuffer(VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkBuffer* outBuffer)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.flags = flags;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = sharingMode;
    return vkCreateBuffer(g_vulkanContextResources.device, &bufferCreateInfo, nullptr, outBuffer);
}

VkResult CreateImage(VkImageCreateFlags flags, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32 mipLevels, uint32 arrayLayers, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, VkImage* outImage)
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = imageType;
    imageCreateInfo.extent = extent;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = arrayLayers;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.usage = usage;
    imageCreateInfo.flags = flags;
    return vkCreateImage(g_vulkanContextResources.device, &imageCreateInfo, nullptr, outImage);
}

static void DbgSetObjectNameBase(uint64 handle, VkObjectType type, const char* name)
{
#ifdef ENABLE_VULKAN_DEBUG_LABELS
    VkDebugUtilsObjectNameInfoEXT info =
    {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        NULL,
        type,
        handle,
        name
    };

    g_vulkanContextResources.pfnSetDebugUtilsObjectNameEXT(g_vulkanContextResources.device, &info);
#endif
}

void DbgSetImageObjectName(uint64 handle, const char* name)
{
    DbgSetObjectNameBase(handle, VK_OBJECT_TYPE_IMAGE, name);
}

void DbgSetBufferObjectName(uint64 handle, const char* name)
{
    DbgSetObjectNameBase(handle, VK_OBJECT_TYPE_BUFFER, name);
}

void DbgStartMarker(VkCommandBuffer commandBuffer, const char* debugLabel)
{
#if defined(ENABLE_VULKAN_DEBUG_LABELS)
    VkDebugUtilsLabelEXT label =
    {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        NULL,
        debugLabel,
        { 0.0f, 0.0f, 0.0f, 0.0f },
    };
    g_vulkanContextResources.pfnCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);
#endif
}

void DbgEndMarker(VkCommandBuffer commandBuffer)
{
#if defined(ENABLE_VULKAN_DEBUG_LABELS)
    g_vulkanContextResources.pfnCmdEndDebugUtilsLabelEXT(commandBuffer);
#endif
}

// NOTE: Must correspond the enums in PlatformGameAPI.h
static VkPipelineColorBlendAttachmentState   VulkanBlendStates     [BlendState::eMax]     = {};
static DepthCullState                        VulkanDepthStates     [DepthState::eMax]     = {};
static VkImageLayout                         VulkanImageLayouts    [ImageLayout::eMax]    = {};
static VkFormat                              VulkanImageFormats    [ImageFormat::eMax]    = {};
static VkDescriptorType                      VulkanDescriptorTypes [DescriptorType::eMax] = {};
static VkBufferUsageFlags                    VulkanBufferUsageFlags[BufferUsage::eMax]    = {};
static VkMemoryPropertyFlagBits              VulkanMemPropertyFlags[BufferUsage::eMax]    = {};

void InitVulkanDataTypesPerEnum()
{
    VkPipelineColorBlendAttachmentState blendStateProperties = {};
    blendStateProperties.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    blendStateProperties.blendEnable = VK_TRUE;
    blendStateProperties.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendStateProperties.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendStateProperties.colorBlendOp = VK_BLEND_OP_ADD;
    blendStateProperties.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendStateProperties.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendStateProperties.alphaBlendOp = VK_BLEND_OP_ADD;
    VulkanBlendStates[BlendState::eAlphaBlend] = blendStateProperties;
    blendStateProperties.blendEnable = VK_FALSE;
    VulkanBlendStates[BlendState::eReplace] = blendStateProperties;
    VulkanBlendStates[BlendState::eNoColorAttachment] = {};

    VkCompareOp depthCompareOps[DepthCompareOp::eMax] = {};
    depthCompareOps[DepthCompareOp::eLeOrEqual] = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthCompareOps[DepthCompareOp::eGeOrEqual] = VK_COMPARE_OP_GREATER_OR_EQUAL;
    TINKER_ASSERT(DEPTH_OP < ARRAYCOUNT(depthCompareOps));

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthCompareOp = depthCompareOps[DEPTH_OP];
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds = DEPTH_MIN;
    depthStencilState.maxDepthBounds = DEPTH_MAX;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = {};
    depthStencilState.back = {};
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    VulkanDepthStates[DepthState::eOff_NoCull].cullMode = VK_CULL_MODE_NONE;
    VulkanDepthStates[DepthState::eOff_CCW].depthState = depthStencilState;
    VulkanDepthStates[DepthState::eOff_CCW].cullMode = VK_CULL_MODE_BACK_BIT;

    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    VulkanDepthStates[DepthState::eTestOnWriteOn_CCW].depthState = depthStencilState;
    VulkanDepthStates[DepthState::eTestOnWriteOn_CCW].cullMode = VK_CULL_MODE_BACK_BIT;

    depthStencilState.depthWriteEnable = VK_FALSE;
    VulkanDepthStates[DepthState::eTestOnWriteOff_CCW].depthState = depthStencilState;
    VulkanDepthStates[DepthState::eTestOnWriteOff_CCW].cullMode = VK_CULL_MODE_BACK_BIT;

    VulkanImageLayouts[ImageLayout::eUndefined] = VK_IMAGE_LAYOUT_UNDEFINED;
    VulkanImageLayouts[ImageLayout::eShaderRead] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VulkanImageLayouts[ImageLayout::eTransferDst] = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    VulkanImageLayouts[ImageLayout::eDepthOptimal] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VulkanImageLayouts[ImageLayout::eRenderOptimal] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VulkanImageLayouts[ImageLayout::ePresent] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VulkanImageFormats[ImageFormat::Invalid] = VK_FORMAT_UNDEFINED;
    VulkanImageFormats[ImageFormat::BGRA8_SRGB] = VK_FORMAT_B8G8R8A8_SRGB;
    VulkanImageFormats[ImageFormat::RGBA8_SRGB] = VK_FORMAT_R8G8B8A8_SRGB;
    VulkanImageFormats[ImageFormat::Depth_32F] = VK_FORMAT_D32_SFLOAT;
    VulkanImageFormats[ImageFormat::TheSwapChainFormat] = g_vulkanContextResources.swapChainFormat;

    VulkanDescriptorTypes[DescriptorType::eBuffer] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VulkanDescriptorTypes[DescriptorType::eSampledImage] = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    VulkanDescriptorTypes[DescriptorType::eSSBO] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VulkanBufferUsageFlags[BufferUsage::eVertex] = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // vertex buffers are actually SSBOs for now
    VulkanBufferUsageFlags[BufferUsage::eIndex] = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VulkanBufferUsageFlags[BufferUsage::eTransientVertex] = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VulkanBufferUsageFlags[BufferUsage::eTransientIndex] = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VulkanBufferUsageFlags[BufferUsage::eStaging] = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VulkanBufferUsageFlags[BufferUsage::eUniform] = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VulkanMemPropertyFlags[BufferUsage::eVertex] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanMemPropertyFlags[BufferUsage::eIndex] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanMemPropertyFlags[BufferUsage::eTransientVertex] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VulkanMemPropertyFlags[BufferUsage::eTransientIndex] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VulkanMemPropertyFlags[BufferUsage::eStaging] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VulkanMemPropertyFlags[BufferUsage::eUniform] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
}

const VkPipelineColorBlendAttachmentState& GetVkBlendState(uint32 gameBlendState)
{
    TINKER_ASSERT(gameBlendState < BlendState::eMax);
    return VulkanBlendStates[gameBlendState];
}

const DepthCullState& GetVkDepthCullState(uint32 gameDepthCullState)
{
    TINKER_ASSERT(gameDepthCullState < DepthState::eMax);
    return VulkanDepthStates[gameDepthCullState];
}

const VkImageLayout& GetVkImageLayout(uint32 gameImageLayout)
{
    TINKER_ASSERT(gameImageLayout < ImageLayout::eMax);
    return VulkanImageLayouts[gameImageLayout];
}

const VkFormat& GetVkImageFormat(uint32 gameImageFormat)
{
    TINKER_ASSERT(gameImageFormat < ImageFormat::eMax);
    return VulkanImageFormats[gameImageFormat];
}

const VkDescriptorType& GetVkDescriptorType(uint32 gameDescriptorType)
{
    TINKER_ASSERT(gameDescriptorType < DescriptorType::eMax);
    return VulkanDescriptorTypes[gameDescriptorType];
}

VkBufferUsageFlags GetVkBufferUsageFlags(uint32 bufferUsage)
{
    TINKER_ASSERT(bufferUsage < BufferUsage::eMax);
    return VulkanBufferUsageFlags[bufferUsage];
}

VkMemoryPropertyFlags GetVkMemoryPropertyFlags(uint32 bufferUsage)
{
    TINKER_ASSERT(bufferUsage < BufferUsage::eMax);
    return VulkanMemPropertyFlags[bufferUsage];
}

}
}

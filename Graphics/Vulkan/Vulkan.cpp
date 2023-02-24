#include "Graphics/Vulkan/Vulkan.h"
#include "Graphics/Vulkan/VulkanTypes.h"
#include "Graphics/Vulkan/VulkanCreation.h"
#include "Utility/Logging.h"

#include <iostream>
// TODO: move this to be a compile define or ini config entry
//#define ENABLE_VULKAN_VALIDATION_LAYERS // enables validation layers

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#endif

#define DEVICE_LOCAL_BUFFER_HEAP_SIZE 512 * 1024 * 1024 // 512 MiB
#define HOST_VISIBLE_HEAP_SIZE 256 * 1024 * 1024 // 120 MiB
#define DEVICE_LOCAL_IMAGE_HEAP_SIZE 512 * 1024 * 1024 // 512 MiB

namespace Tk
{
namespace Graphics
{

static const uint32 VULKAN_SCRATCH_MEM_SIZE = 1024 * 1024 * 2;

#if defined(ENABLE_VULKAN_VALIDATION_LAYERS)
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallbackFunc(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) | (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
    {
        OutputDebugString(pCallbackData->pMessage);
        OutputDebugString("\n");
    }

    return VK_FALSE;
}
#endif

// This code exists so that we can preallocate proper VkDeviceMemory's before creating any buffers for faster GPU memory allocation
static void InitGPUMemAllocators()
{
    // TODO don't do this a second time here
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(g_vulkanContextResources.physicalDevice, &properties);

    VkMemoryRequirements memRequirements = {};

    {
        // Create test buffer to query memory requirements
        VkBufferUsageFlags AllPossibleFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkBuffer TestBuffer = VK_NULL_HANDLE;
        VkResult result = CreateBuffer(0, 64u, AllPossibleFlags, VK_SHARING_MODE_EXCLUSIVE, &TestBuffer);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to create buffer!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
        }

        vkGetBufferMemoryRequirements(g_vulkanContextResources.device, TestBuffer, &memRequirements);
        // Done with the buffer
        vkDestroyBuffer(g_vulkanContextResources.device, TestBuffer, nullptr);

        uint32 memoryTypeIndex = ChooseMemoryTypeBits(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        g_vulkanContextResources.GPUMemAllocators[g_vulkanContextResources.eVulkanMemoryAllocatorDeviceLocalBuffers].Init(DEVICE_LOCAL_BUFFER_HEAP_SIZE, memoryTypeIndex, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, properties.limits.nonCoherentAtomSize);
    }

    {
        VkBufferUsageFlags AllPossibleFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkBuffer TestBuffer = VK_NULL_HANDLE;
        VkResult result = CreateBuffer(0, 64u, AllPossibleFlags, VK_SHARING_MODE_EXCLUSIVE, &TestBuffer);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to create buffer!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
        }

        vkGetBufferMemoryRequirements(g_vulkanContextResources.device, TestBuffer, &memRequirements);
        // Done with the buffer
        vkDestroyBuffer(g_vulkanContextResources.device, TestBuffer, nullptr);

        uint32 memoryTypeIndex = ChooseMemoryTypeBits(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        g_vulkanContextResources.GPUMemAllocators[g_vulkanContextResources.eVulkanMemoryAllocatorHostVisibleBuffers].Init(HOST_VISIBLE_HEAP_SIZE, memoryTypeIndex, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, properties.limits.nonCoherentAtomSize);

        // Note: special!!! this memory block is going to be persistently mapped for now
        result = vkMapMemory(g_vulkanContextResources.device, g_vulkanContextResources.GPUMemAllocators[g_vulkanContextResources.eVulkanMemoryAllocatorHostVisibleBuffers].m_GPUMemory, 0, VK_WHOLE_SIZE, 0, &g_vulkanContextResources.GPUMemAllocators[g_vulkanContextResources.eVulkanMemoryAllocatorHostVisibleBuffers].m_MappedMemPtr);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to map gpu memory!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
        }
    }

    {
        VkImageUsageFlags AllPossibleFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImage TestImage = VK_NULL_HANDLE;
        VkResult result = CreateImage(0, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, { 64u, 64u, 1u }, 6u, 4u, VK_IMAGE_TILING_OPTIMAL, AllPossibleFlags, VK_SHARING_MODE_EXCLUSIVE, &TestImage);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to create image!", Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
        }

        vkGetImageMemoryRequirements(g_vulkanContextResources.device, TestImage, &memRequirements);
        // Done with the image
        vkDestroyImage(g_vulkanContextResources.device, TestImage, nullptr);

        uint32 memoryTypeIndex = ChooseMemoryTypeBits(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        g_vulkanContextResources.GPUMemAllocators[g_vulkanContextResources.eVulkanMemoryAllocatorDeviceLocalImages].Init(DEVICE_LOCAL_IMAGE_HEAP_SIZE, memoryTypeIndex, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, properties.limits.nonCoherentAtomSize);
    }
}

int InitVulkan(const Tk::Platform::WindowHandles* platformWindowHandles, uint32 width, uint32 height)
{
    g_vulkanContextResources.DataAllocator.Init(VULKAN_SCRATCH_MEM_SIZE, 1);

    g_vulkanContextResources.vulkanMemResourcePool.Init(VULKAN_RESOURCE_POOL_MAX, 16);
    g_vulkanContextResources.vulkanDescriptorResourcePool.Init(VULKAN_RESOURCE_POOL_MAX, 16);

    g_vulkanContextResources.windowWidth = width;
    g_vulkanContextResources.windowHeight = height;

    // Init shader pso permutations
    for (uint32 sid = 0; sid < VulkanContextResources::eMaxShaders; ++sid)
    {
        g_vulkanContextResources.psoPermutations.pipelineLayout[sid] = VK_NULL_HANDLE;

        for (uint32 bs = 0; bs < VulkanContextResources::eMaxBlendStates; ++bs)
        {
            for (uint32 ds = 0; ds < VulkanContextResources::eMaxDepthStates; ++ds)
            {
                g_vulkanContextResources.psoPermutations.graphicsPipeline[sid][bs][ds] = VK_NULL_HANDLE;
            }
        }
    }
    //-----

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Tinker with Vulkan";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Tinker Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    // Instance extensions
    const char* enabledExtensionNames[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,

        #if defined(_WIN32)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #else
        // TODO: different platform surface extension
        #endif
        
        #if defined(ENABLE_VULKAN_VALIDATION_LAYERS) || defined(ENABLE_VULKAN_DEBUG_LABELS)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        #endif
    };
    const uint32 numEnabledExtensions = ARRAYCOUNT(enabledExtensionNames);

    Core::Utility::LogMsg("Platform", "******** Requested Instance Extensions: ********", Core::Utility::LogSeverity::eInfo);
    for (uint32 uiReqExt = 0; uiReqExt < numEnabledExtensions; ++uiReqExt)
    {
        Core::Utility::LogMsg("Platform", enabledExtensionNames[uiReqExt], Core::Utility::LogSeverity::eInfo);
    }
    
    instanceCreateInfo.enabledExtensionCount = numEnabledExtensions;
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensionNames;

    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    
    // Get the available extensions
    uint32 numAvailableExtensions = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, nullptr);
    
    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)g_vulkanContextResources.DataAllocator.Alloc(sizeof(VkExtensionProperties) * numAvailableExtensions, 1);
    vkEnumerateInstanceExtensionProperties(nullptr, &numAvailableExtensions, availableExtensions);
    //Core::Utility::LogMsg("Platform", "******** Available Instance Extensions: ********", Core::Utility::LogSeverity::eInfo);
    //for (uint32 uiAvailExt = 0; uiAvailExt < numAvailableExtensions; ++uiAvailExt)
    //{
        //Core::Utility::LogMsg("Platform", availableExtensions[uiAvailExt].extensionName, Core::Utility::LogSeverity::eInfo);
    //}

    // Instance layers
    /*const char* requestedLayersStr[] =
    {
        #if defined(ENABLE_VULKAN_VALIDATION_LAYERS)
        "VK_LAYER_KHRONOS_validation",
        #endif
    };*/
    const uint32 numRequestedLayers = 0;// ARRAYCOUNT(requestedLayersStr);

    /*Core::Utility::LogMsg("Platform", "******** Requested Instance Layers: ********", Core::Utility::LogSeverity::eInfo);
    for (uint32 uiReqLayer = 0; uiReqLayer < numRequestedLayers; ++uiReqLayer)
    {
        Core::Utility::LogMsg("Platform", requestedLayersStr[uiReqLayer], Core::Utility::LogSeverity::eInfo);
    }*/

    uint32 numAvailableLayers = 0;
    vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);

    if (numAvailableLayers == 0)
    {
        Core::Utility::LogMsg("Platform", "Zero available instance layers!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    VkLayerProperties* availableLayers = (VkLayerProperties*)g_vulkanContextResources.DataAllocator.Alloc(sizeof(VkLayerProperties) * numAvailableLayers, 1);
    vkEnumerateInstanceLayerProperties(&numAvailableLayers, availableLayers);

    //Core::Utility::LogMsg("Platform", "******** Available Instance Layers: ********", Core::Utility::LogSeverity::eInfo);
    //for (uint32 uiAvailLayer = 0; uiAvailLayer < numAvailableLayers; ++uiAvailLayer)
    //{
        //Core::Utility::LogMsg("Platform", availableLayers[uiAvailLayer].layerName, Core::Utility::LogSeverity::eInfo);
    //}

    /*uint32 layersSupported[numRequestedLayers] = {};
    for (uint32 uiReqLayer = 0; uiReqLayer < numRequestedLayers; ++uiReqLayer)
    {
        for (uint32 uiAvailLayer = 0; uiAvailLayer < numAvailableLayers; ++uiAvailLayer)
        {
            if (!strcmp(availableLayers[uiAvailLayer].layerName, requestedLayersStr[uiReqLayer]))
            {
                layersSupported[uiReqLayer] = 1u;
                break;
            }
        }
    }

    for (uint32 uiReqLayer = 0; uiReqLayer < numRequestedLayers; ++uiReqLayer)
    {
        if (!layersSupported[uiReqLayer])
        {
            Core::Utility::LogMsg("Platform", "Requested instance layer not supported!", Core::Utility::LogSeverity::eCritical);
            Core::Utility::LogMsg("Platform", requestedLayersStr[uiReqLayer], Core::Utility::LogSeverity::eCritical);
            TINKER_ASSERT(0);
        }
    }*/

    instanceCreateInfo.enabledLayerCount = 0;// numRequestedLayers;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;// requestedLayersStr;

    VkResult result = vkCreateInstance(&instanceCreateInfo,
        nullptr,
        &g_vulkanContextResources.instance);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to create Vulkan instance!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    #if defined(ENABLE_VULKAN_VALIDATION_LAYERS)
    // Debug utils callback
    VkDebugUtilsMessengerCreateInfoEXT dbgUtilsMsgCreateInfo = {};
    dbgUtilsMsgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbgUtilsMsgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbgUtilsMsgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbgUtilsMsgCreateInfo.pfnUserCallback = DebugCallbackFunc;
    dbgUtilsMsgCreateInfo.pUserData = nullptr;

    PFN_vkCreateDebugUtilsMessengerEXT dbgCreateFunc =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_vulkanContextResources.instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");
    if (dbgCreateFunc)
    {
        dbgCreateFunc(g_vulkanContextResources.instance,
            &dbgUtilsMsgCreateInfo,
            nullptr,
            &g_vulkanContextResources.debugMessenger);
    }
    else
    {
        Core::Utility::LogMsg("Platform", "Failed to get create debug utils messenger proc addr!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
    #endif

    // Surface
    #if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = {};
    win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfo.hinstance = (HINSTANCE)platformWindowHandles->procInstHandle;
    win32SurfaceCreateInfo.hwnd = (HWND)platformWindowHandles->windowInstHandle;

    result = vkCreateWin32SurfaceKHR(g_vulkanContextResources.instance,
        &win32SurfaceCreateInfo,
        NULL,
        &g_vulkanContextResources.surface);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to create Win32SurfaceKHR!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
    #else
    // TODO: implement other platform surface types
    TINKER_ASSERT(0);
    #endif

    // Physical device
    uint32 numPhysicalDevices = 0;
    vkEnumeratePhysicalDevices(g_vulkanContextResources.instance, &numPhysicalDevices, nullptr);

    if (numPhysicalDevices == 0)
    {
        Core::Utility::LogMsg("Platform", "Zero available physical devices!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)g_vulkanContextResources.DataAllocator.Alloc(sizeof(VkPhysicalDevice) * numPhysicalDevices, 1);
    vkEnumeratePhysicalDevices(g_vulkanContextResources.instance, &numPhysicalDevices, physicalDevices);

    const uint32 numRequiredPhysicalDeviceExtensions = 1;
    const char* requiredPhysicalDeviceExtensions[numRequiredPhysicalDeviceExtensions] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    Core::Utility::LogMsg("Platform", "******** Requested Device Extensions: ********", Core::Utility::LogSeverity::eInfo);
    for (uint32 uiReqExt = 0; uiReqExt < numRequiredPhysicalDeviceExtensions; ++uiReqExt)
    {
        Core::Utility::LogMsg("Platform", requiredPhysicalDeviceExtensions[uiReqExt], Core::Utility::LogSeverity::eInfo);
    }

    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features = {};

    for (uint32 uiPhysicalDevice = 0; uiPhysicalDevice < numPhysicalDevices; ++uiPhysicalDevice)
    {
        VkPhysicalDevice currPhysicalDevice = physicalDevices[uiPhysicalDevice];

        physicalDeviceProperties = {};
        vkGetPhysicalDeviceProperties(currPhysicalDevice, &physicalDeviceProperties);

        physicalDeviceFeatures2 = {};
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceVulkan13Features = {};
        physicalDeviceVulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        physicalDeviceFeatures2.pNext = &physicalDeviceVulkan13Features;
        vkGetPhysicalDeviceFeatures2(currPhysicalDevice, &physicalDeviceFeatures2);
        
        // Required device feature - can't use this device if not available
        if (physicalDeviceVulkan13Features.dynamicRendering == VK_FALSE)
        {
            continue;
        }

        // Required, push constant minimum size
        if (physicalDeviceProperties.limits.maxPushConstantsSize < MIN_PUSH_CONSTANTS_SIZE)
        {
            continue;
        }

        if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            // Queue family
            uint32 numQueueFamilies = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &numQueueFamilies, nullptr);

            VkQueueFamilyProperties* queueFamilyProperties = (VkQueueFamilyProperties*)g_vulkanContextResources.DataAllocator.Alloc(sizeof(VkQueueFamilyProperties) * numQueueFamilies, 1);
            vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &numQueueFamilies, queueFamilyProperties);

            bool graphicsSupport = false;
            bool presentationSupport = false;
            bool extensionSupport[numRequiredPhysicalDeviceExtensions] = { false };

            // Check queue family properties
            for (uint32 uiQueueFamily = 0; uiQueueFamily < numQueueFamilies; ++uiQueueFamily)
            {
                if (queueFamilyProperties[uiQueueFamily].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    graphicsSupport = true;
                    presentationSupport = true;
                    g_vulkanContextResources.graphicsQueueIndex = uiQueueFamily;
                }
            }

            uint32 numAvailablePhysicalDeviceExtensions = 0;
            vkEnumerateDeviceExtensionProperties(currPhysicalDevice,
                nullptr,
                &numAvailablePhysicalDeviceExtensions,
                nullptr);

            if (numAvailablePhysicalDeviceExtensions == 0)
            {
                Core::Utility::LogMsg("Platform", "Zero available device extensions!", Core::Utility::LogSeverity::eCritical);
                TINKER_ASSERT(0);
            }

            VkExtensionProperties* availablePhysicalDeviceExtensions = (VkExtensionProperties*)g_vulkanContextResources.DataAllocator.Alloc(sizeof(VkExtensionProperties) * numAvailablePhysicalDeviceExtensions, 1);
            vkEnumerateDeviceExtensionProperties(currPhysicalDevice,
                nullptr,
                &numAvailablePhysicalDeviceExtensions,
                availablePhysicalDeviceExtensions);

            //Core::Utility::LogMsg("Platform", "******** Available Device Extensions: ********", Core::Utility::LogSeverity::eInfo);
            //for (uint32 uiAvailExt = 0; uiAvailExt < numAvailablePhysicalDeviceExtensions; ++uiAvailExt)
            //{
                //Core::Utility::LogMsg("Platform", availablePhysicalDeviceExtensions[uiAvailExt].extensionName, Core::Utility::LogSeverity::eInfo);
            //}

            for (uint32 uiReqExt = 0; uiReqExt < numRequiredPhysicalDeviceExtensions; ++uiReqExt)
            {
                for (uint32 uiAvailExt = 0; uiAvailExt < numAvailablePhysicalDeviceExtensions; ++uiAvailExt)
                {
                    if (!strcmp(availablePhysicalDeviceExtensions[uiAvailExt].extensionName, requiredPhysicalDeviceExtensions[uiReqExt]))
                    {
                        extensionSupport[uiReqExt] = true;
                        break;
                    }
                }
            }

            for (uint32 uiReqExt = 0; uiReqExt < numRequiredPhysicalDeviceExtensions; ++uiReqExt)
            {
                if (!extensionSupport[uiReqExt])
                {
                    Core::Utility::LogMsg("Platform", "Requested device extension not supported!", Core::Utility::LogSeverity::eCritical);
                    Core::Utility::LogMsg("Platform", requiredPhysicalDeviceExtensions[uiReqExt], Core::Utility::LogSeverity::eCritical);
                    TINKER_ASSERT(0);
                }
            }

            // Extension support assumed at this point
            if (graphicsSupport && presentationSupport)
            {
                // Select the current physical device
                g_vulkanContextResources.physicalDevice = currPhysicalDevice;
                break;
            }
        }
    }

    if (g_vulkanContextResources.physicalDevice == VK_NULL_HANDLE)
    {
        Core::Utility::LogMsg("Platform", "No physical device chosen!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    const bool timestampsAvailable = physicalDeviceProperties.limits.timestampComputeAndGraphics;
    if (!timestampsAvailable)
    {
        Core::Utility::LogMsg("Graphics", "Timestamps not supported on this device", Core::Utility::LogSeverity::eInfo);
    }
    else
    {
        // supported, need to query the period since it is not the same across vendors
        g_vulkanContextResources.timestampPeriod = physicalDeviceProperties.limits.timestampPeriod;
    }

    // Physical device memory heaps
    if (0)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties = {};
        vkGetPhysicalDeviceMemoryProperties(g_vulkanContextResources.physicalDevice, &memoryProperties);
        {
            Core::Utility::LogMsg("Platform", "******** Device Memory Properties: ********", Core::Utility::LogSeverity::eInfo);

            for (uint32 uiMemType = 0; uiMemType < memoryProperties.memoryTypeCount; ++uiMemType)
            {
                Core::Utility::LogMsg("Platform", "****************", Core::Utility::LogSeverity::eInfo);

                // Heap properties
                // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryPropertyFlagBits.html
                VkMemoryPropertyFlags propertyFlags = memoryProperties.memoryTypes[uiMemType].propertyFlags;

                Core::Utility::LogMsg("Platform", "Heap Properties:", Core::Utility::LogSeverity::eInfo);
                if (propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                {
                    Core::Utility::LogMsg("Platform", "- Device local", Core::Utility::LogSeverity::eInfo);
                }
                if (propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                {
                    Core::Utility::LogMsg("Platform", "- Host visible", Core::Utility::LogSeverity::eInfo);
                }
                if (propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                {
                    Core::Utility::LogMsg("Platform", "- Host coherent", Core::Utility::LogSeverity::eInfo);
                }
                if (propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                {
                    Core::Utility::LogMsg("Platform", "- Host cached", Core::Utility::LogSeverity::eInfo);
                }
                if (propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                {
                    Core::Utility::LogMsg("Platform", "- Lazily allocated", Core::Utility::LogSeverity::eInfo);
                }

                // Heap size
                uint32 heapIndex = memoryProperties.memoryTypes[uiMemType].heapIndex;
                char buffer[16] = {};
                _ui64toa_s(memoryProperties.memoryHeaps[heapIndex].size, buffer, 16, 10);

                Core::Utility::LogMsg("Platform", "Heap Size:", Core::Utility::LogSeverity::eInfo);
                Core::Utility::LogMsg("Platform", buffer, Core::Utility::LogSeverity::eInfo);

                // Heap flags
                // memoryProperties.memoryHeaps[heapIndex].flags
                // Don't really care about these I think
                // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryHeapFlagBits.html
            }
            Core::Utility::LogMsg("Platform", "****************", Core::Utility::LogSeverity::eInfo);
        }
    }

    // Logical device
    const uint32 numQueues = 1;
    VkDeviceQueueCreateInfo deviceQueueCreateInfos[numQueues] = {};

    // Create graphics queue
    deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[0].queueFamilyIndex = g_vulkanContextResources.graphicsQueueIndex;
    deviceQueueCreateInfos[0].queueCount = 1;
    float graphicsQueuePriority = 1.0f;
    deviceQueueCreateInfos[0].pQueuePriorities = &graphicsQueuePriority;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
    deviceCreateInfo.queueCreateInfoCount = numQueues;
    VkPhysicalDeviceFeatures requestedPhysicalDeviceFeatures = {};
    // TODO: request physical device features
    deviceCreateInfo.pEnabledFeatures = &requestedPhysicalDeviceFeatures;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = numRequiredPhysicalDeviceExtensions;
    deviceCreateInfo.ppEnabledExtensionNames = requiredPhysicalDeviceExtensions;
    physicalDeviceVulkan13Features.dynamicRendering = VK_TRUE;
    deviceCreateInfo.pNext = &physicalDeviceVulkan13Features;

    result = vkCreateDevice(g_vulkanContextResources.physicalDevice,
        &deviceCreateInfo,
        nullptr,
        &g_vulkanContextResources.device);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to create Vulkan device!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    // Debug utils labels / debug markers
    #if defined(ENABLE_VULKAN_DEBUG_LABELS)
    g_vulkanContextResources.pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(g_vulkanContextResources.device, "vkCmdBeginDebugUtilsLabelEXT");
    g_vulkanContextResources.pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(g_vulkanContextResources.device, "vkCmdEndDebugUtilsLabelEXT");
    g_vulkanContextResources.pfnCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(g_vulkanContextResources.device, "vkCmdInsertDebugUtilsLabelEXT");
    g_vulkanContextResources.pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(g_vulkanContextResources.device, "vkSetDebugUtilsObjectNameEXT");

    if (!g_vulkanContextResources.pfnCmdBeginDebugUtilsLabelEXT ||
        !g_vulkanContextResources.pfnCmdEndDebugUtilsLabelEXT ||
        !g_vulkanContextResources.pfnCmdInsertDebugUtilsLabelEXT ||
        !g_vulkanContextResources.pfnSetDebugUtilsObjectNameEXT
        )
    {
        Core::Utility::LogMsg("Platform", "Failed to get create debug utils marker proc addr!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
    #endif

    // Queues
    vkGetDeviceQueue(g_vulkanContextResources.device,
        g_vulkanContextResources.graphicsQueueIndex,
        0,
        &g_vulkanContextResources.graphicsQueue);

    // Swap chain
    VulkanCreateSwapChain();

    // Command pool
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = g_vulkanContextResources.graphicsQueueIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    result = vkCreateCommandPool(g_vulkanContextResources.device,
        &commandPoolCreateInfo,
        nullptr,
        &g_vulkanContextResources.commandPool);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to create Vulkan command pool!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    // Command buffers for per-frame submission
    g_vulkanContextResources.commandBuffers = (VkCommandBuffer*)g_vulkanContextResources.DataAllocator.Alloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT, 1);

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = g_vulkanContextResources.commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    result = vkAllocateCommandBuffers(g_vulkanContextResources.device,
        &commandBufferAllocInfo,
        g_vulkanContextResources.commandBuffers);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to allocate Vulkan primary frame command buffers!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    // Command buffer for immediate submission
    commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandPool = g_vulkanContextResources.commandPool;
    commandBufferAllocInfo.commandBufferCount = 1;
    result = vkAllocateCommandBuffers(g_vulkanContextResources.device, &commandBufferAllocInfo, &g_vulkanContextResources.commandBuffer_Immediate);
    if (result != VK_SUCCESS)
    {
        Core::Utility::LogMsg("Platform", "Failed to allocate Vulkan command buffers (immediate)!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }

    // Virtual frame synchronization data initialization - 2 semaphores and fence
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32 uiFrame = 0; uiFrame < MAX_FRAMES_IN_FLIGHT; ++uiFrame)
    {
        result = vkCreateSemaphore(g_vulkanContextResources.device,
            &semaphoreCreateInfo,
            nullptr,
            &g_vulkanContextResources.virtualFrameSyncData[uiFrame].GPUWorkCompleteSema);

        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to create Vulkan gpu work complete semaphore!", Core::Utility::LogSeverity::eCritical);
        }

        result = vkCreateSemaphore(g_vulkanContextResources.device,
            &semaphoreCreateInfo,
            nullptr,
            &g_vulkanContextResources.virtualFrameSyncData[uiFrame].ImageAvailableSema);

        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to create Vulkan present semaphore!", Core::Utility::LogSeverity::eCritical);
        }
    }

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32 uiFrame = 0; uiFrame < MAX_FRAMES_IN_FLIGHT; ++uiFrame)
    {
        result = vkCreateFence(g_vulkanContextResources.device, &fenceCreateInfo, nullptr, &g_vulkanContextResources.virtualFrameSyncData[uiFrame].Fence);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Platform", "Failed to create Vulkan fence!", Core::Utility::LogSeverity::eCritical);
        }
    }

    // Timestamp query pool
    if (timestampsAvailable)
    {
        const uint32 timestampQueryCount = MAX_FRAMES_IN_FLIGHT * GPU_TIMESTAMP_NUM_MAX;

        VkQueryPoolCreateInfo queryPoolCreateInfo = {};
        queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        queryPoolCreateInfo.pNext = NULL;
        queryPoolCreateInfo.flags = (VkQueryPoolCreateFlags)0;
        queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        queryPoolCreateInfo.queryCount = timestampQueryCount;
        queryPoolCreateInfo.pipelineStatistics = 0;

        result = vkCreateQueryPool(g_vulkanContextResources.device, &queryPoolCreateInfo, NULL, &g_vulkanContextResources.queryPoolTimestamp);
        if (result != VK_SUCCESS)
        {
            Core::Utility::LogMsg("Graphics", "Failed to timestamp query pool!", Core::Utility::LogSeverity::eCritical);
        }
    }
    
    CreateSamplers();

    InitVulkanDataTypesPerEnum();

    InitGPUMemAllocators();

    g_vulkanContextResources.isInitted = true;
    return 0;
}

void DestroyVulkan()
{
    if (!g_vulkanContextResources.isInitted)
        return;

    vkDeviceWaitIdle(g_vulkanContextResources.device); // TODO: move this?

    vkDestroyQueryPool(g_vulkanContextResources.device, g_vulkanContextResources.queryPoolTimestamp, nullptr);

    VulkanDestroySwapChain();

    vkDestroyCommandPool(g_vulkanContextResources.device, g_vulkanContextResources.commandPool, nullptr);
    g_vulkanContextResources.commandBuffers = nullptr;

    VulkanDestroyAllPSOPerms();
    DestroyAllDescLayouts();

    for (uint32 uiFrame = 0; uiFrame < MAX_FRAMES_IN_FLIGHT; ++uiFrame)
    {
        vkDestroySemaphore(g_vulkanContextResources.device, g_vulkanContextResources.virtualFrameSyncData[uiFrame].GPUWorkCompleteSema, nullptr);
        vkDestroySemaphore(g_vulkanContextResources.device, g_vulkanContextResources.virtualFrameSyncData[uiFrame].ImageAvailableSema, nullptr);
        vkDestroyFence(g_vulkanContextResources.device, g_vulkanContextResources.virtualFrameSyncData[uiFrame].Fence, nullptr);
    }

    vkDestroySampler(g_vulkanContextResources.device, g_vulkanContextResources.linearSampler, nullptr);

    for (uint32 uiAlloc = 0; uiAlloc < ARRAYCOUNT(g_vulkanContextResources.GPUMemAllocators); ++uiAlloc)
    {
        g_vulkanContextResources.GPUMemAllocators[uiAlloc].Destroy();
    }

    #if defined(ENABLE_VULKAN_VALIDATION_LAYERS)
    // Debug utils messenger
    PFN_vkDestroyDebugUtilsMessengerEXT dbgDestroyFunc =
        (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(g_vulkanContextResources.instance,
                                                                    "vkDestroyDebugUtilsMessengerEXT");
    if (dbgDestroyFunc)
    {
        dbgDestroyFunc(g_vulkanContextResources.instance, g_vulkanContextResources.debugMessenger, nullptr);
    }
    else
    {
        Core::Utility::LogMsg("Platform", "Failed to get destroy debug utils messenger proc addr!", Core::Utility::LogSeverity::eCritical);
        TINKER_ASSERT(0);
    }
    #endif

    vkDestroyDevice(g_vulkanContextResources.device, nullptr);
    vkDestroySurfaceKHR(g_vulkanContextResources.instance, g_vulkanContextResources.surface, nullptr);
    vkDestroyInstance(g_vulkanContextResources.instance, nullptr);

    g_vulkanContextResources.vulkanMemResourcePool.ExplicitFree();
    g_vulkanContextResources.vulkanDescriptorResourcePool.ExplicitFree();
}

float GetGPUTimestampPeriod()
{
    return g_vulkanContextResources.timestampPeriod;
}

uint32 GetCurrentFrameInFlightIndex()
{
    return g_vulkanContextResources.currentVirtualFrame;
}

}
}

#pragma once

#include "Math/VectorTypes.h"

namespace Tk
{

namespace Platform
{
struct WindowHandles;
}

struct GraphicsCommandStream;

namespace Graphics
{

// Enums
namespace DescriptorType
{
    enum : uint32
    {
        eBuffer = 0,
        eSampledImage,
        eSSBO,
        eMax
    };
}

namespace BufferUsage
{
    enum : uint32
    {
        eVertex = 0,
        eIndex,
        eTransientVertex,
        eTransientIndex,
        eStaging,
        eUniform,
        eMax
    };
}

namespace ResourceType
{
    enum : uint32
    {
        eBuffer1D = 0,
        eImage2D,
        eMax
    };
}

namespace BlendState
{
    enum : uint32
    {
        eAlphaBlend = 0,
        eReplace,
        eNoColorAttachment,
        eMax
    };
}

namespace DepthState
{
     enum : uint32
     {
         eOff_CCW = 0,
         eOff_NoCull,
         eTestOnWriteOn_CCW,
         eTestOnWriteOff_CCW,
         eMax
     };   
}

namespace ImageFormat
{
    enum : uint32
    {
        Invalid = 0,
        BGRA8_SRGB,
        RGBA8_SRGB,
        Depth_32F,
        TheSwapChainFormat,
        eMax
    };
}

namespace ImageLayout
{
    enum : uint32
    {
        eUndefined = 0,
        eShaderRead,
        eTransferDst,
        eRenderOptimal,
        eDepthOptimal,
        ePresent,
        eMax
    };
}

namespace DepthCompareOp
{
    enum : uint32
    {
        eLeOrEqual = 0,
        eGeOrEqual,
        // TODO: support strictly less than for normal rendering?
        eMax
    };
}

extern uint32 MultiBufferedStatusFromBufferUsage[BufferUsage::eMax];
inline uint32 IsBufferUsageMultiBuffered(uint32 bufferUsage)
{
    TINKER_ASSERT(bufferUsage < BufferUsage::eMax);
    return MultiBufferedStatusFromBufferUsage[bufferUsage];
}

// Concrete type for resource handle to catch errors at compile time, e.g.
// Try to free a descriptor set with a resource handle, which can happen if all handles
// are just plain uint32.
struct ResourceHandle
{
    uint32 m_hRes;

    ResourceHandle()
    {
        m_hRes = TINKER_INVALID_HANDLE;
    }

    // Warning: probably don't pass around handles as uint32 willy-nilly
    explicit ResourceHandle(uint32 h)
    {
        m_hRes = h;
    }

    inline bool operator==(const ResourceHandle& other) const
    {
        return m_hRes == other.m_hRes;
    }

    inline bool operator!=(const ResourceHandle& other) const
    {
        return m_hRes != other.m_hRes;
    }
};
#define DefaultResHandle_Invalid ResourceHandle()

typedef struct graphics_resource_description
{
    v3ui dims = {};
    uint32 resourceType = ResourceType::eMax;

    union
    {
        // Buffers
        uint32 bufferUsage;

        // Images
        struct
        {
            uint32 imageFormat;
            uint32 arrayEles;
        };
    };

    const char* debugLabel = "";
} ResourceDesc;

struct FramebufferHandle
{
    uint32 m_hFramebuffer;

    FramebufferHandle()
    {
        m_hFramebuffer = TINKER_INVALID_HANDLE;
    }

    // Warning: probably don't pass around handles as uint32 willy-nilly
    explicit FramebufferHandle(uint32 h)
    {
        m_hFramebuffer = h;
    }

    inline bool operator==(const FramebufferHandle& other) const
    {
        return m_hFramebuffer == other.m_hFramebuffer;
    }

    inline bool operator!=(const FramebufferHandle& other) const
    {
        return m_hFramebuffer != other.m_hFramebuffer;
    }
};
#define DefaultFramebufferHandle_Invalid FramebufferHandle()

struct DescriptorHandle
{
    uint32 m_hDesc;

    DescriptorHandle()
    {
        m_hDesc = TINKER_INVALID_HANDLE;
    }

    // Warning: probably don't pass around handles as uint32 willy-nilly
    explicit DescriptorHandle(uint32 h)
    {
        m_hDesc = h;
    }

    inline bool operator==(const DescriptorHandle& other) const
    {
        return m_hDesc == other.m_hDesc;
    }

    inline bool operator!=(const DescriptorHandle& other) const
    {
        return m_hDesc != other.m_hDesc;
    }
};
#define DefaultDescHandle_Invalid DescriptorHandle()

#define MAX_DESCRIPTOR_SETS_PER_SHADER 4
#define MAX_BINDINGS_PER_SET 3

typedef struct descriptor_layout_params
{
    uint32 type;
    uint32 amount;
} DescriptorLayoutParams;

typedef struct descriptor_layout
{
    DescriptorLayoutParams params[MAX_BINDINGS_PER_SET];

    void InitInvalid()
    {
        for (uint32 uiDesc = 0; uiDesc < MAX_BINDINGS_PER_SET; ++uiDesc)
        {
            params[uiDesc].type = DescriptorType::eMax;
            params[uiDesc].amount = 0;
        }
    }
} DescriptorLayout;

// list of resource handles in a descriptor set
typedef struct descriptor_set_data_handles
{
    ResourceHandle handles[MAX_BINDINGS_PER_SET];

    void InitInvalid()
    {
        for (uint32 uiDesc = 0; uiDesc < MAX_BINDINGS_PER_SET; ++uiDesc)
        {
            handles[uiDesc] = DefaultResHandle_Invalid;
        }
    }
} DescriptorSetDataHandles;

// Important graphics defines
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_MULTIPLE_RENDERTARGETS 8u

#define IMAGE_HANDLE_SWAP_CHAIN ResourceHandle(0xFFFFFFFE) // INVALID_HANDLE - 1 reserved to refer to the swap chain image 

//#define DEPTH_REVERSED
#ifndef DEPTH_REVERSED
#define DEPTH_MIN 0.0f
#define DEPTH_MAX 1.0f
#define DEPTH_OP DepthCompareOp::eLeOrEqual
#else
// TODO: untested
#define DEPTH_MIN 1.0f
#define DEPTH_MAX 0.0f
#define DEPTH_OP DepthCompareOp::eGeOrEqual
#endif

#define GPU_TIMESTAMP_NUM_MAX 5

#define MIN_PUSH_CONSTANTS_SIZE 128 // bytes

typedef struct graphics_command
{
    enum : uint32
    {
        eDrawCall = 0,
        eMemTransfer,
        ePushConstant,
        eSetScissor,
        eRenderPassBegin,
        eRenderPassEnd,
        eLayoutTransition,
        eClearImage,
        //eImageCopy,
        eGPUTimestamp,
        eMax
    };

    const char* debugLabel = "Default Label";
    uint32 m_commandType;

    union
    {
        // Draw call
        struct
        {
            uint32 m_numIndices;
            uint32 m_numInstances;
            uint32 m_vertOffset;
            uint32 m_indexOffset;
            uint32 m_shader;
            uint32 m_blendState;
            uint32 m_depthState;
            ResourceHandle m_indexBufferHandle;
            DescriptorHandle m_descriptors[MAX_DESCRIPTOR_SETS_PER_SHADER];
        };

        // Memory transfer
        struct
        {
            uint32 m_sizeInBytes;
            ResourceHandle m_srcBufferHandle;
            ResourceHandle m_dstBufferHandle;
        };

        // Push constant
        struct
        {
            uint32 m_shaderForLayout;
            // TODO: more data here makes this struct bigger, maybe store a pointer?
            uint8 m_pushConstantData[32];
        };

        // Scissor
        struct
        {
            int32 m_scissorOffsetX;
            int32 m_scissorOffsetY;
            uint32 m_scissorWidth;
            uint32 m_scissorHeight;
        };

        // Begin render pass
        struct
        {
            uint32 m_renderWidth;
            uint32 m_renderHeight;
            uint32 m_numColorRTs;
            ResourceHandle m_colorRTs[MAX_MULTIPLE_RENDERTARGETS];
            ResourceHandle m_depthRT;
        };

        // End render pass
        // NOTE: no actual data required
        /* struct
        {
        }; */

        // Image Layout Transition
        struct
        {
            ResourceHandle m_imageHandle;
            uint32 m_startLayout;
            uint32 m_endLayout;
        };

        // Clear image
        struct
        {
            ResourceHandle m_imageHandle;
            v4f m_clearValue;
        };

        // Image copy
        /*struct
        {
            uint32 m_width;
            uint32 m_height;
            ResourceHandle m_srcImgHandle;
            ResourceHandle m_dstImgHandle;
        };*/

        // GPU Timestamp
        struct
        {
            const char* m_timestampNameStr;
            bool m_timestampStartFrame;
        };
    };

    void CmdTimestamp(const char* nameStr, const char* dbgLabel = "Timestamp", bool startFrame = false)
    {
        m_commandType = eGPUTimestamp;
        debugLabel = dbgLabel;
        m_timestampNameStr = nameStr;
        m_timestampStartFrame = startFrame;
    }

} GraphicsCommand;

struct GraphicsCommandStream
{
    GraphicsCommand* m_graphicsCommands;
    uint32 m_numCommands;
    uint32 m_maxCommands;
};


// TODO: move all this, and also autogenerate this eventually?
// TODO: don't use them as uint32's 
// IDs must be uniquely named and have their id ascend monotonically from 0
enum
{
    DESCLAYOUT_ID_SWAP_CHAIN_BLIT_TEX = 0,
    DESCLAYOUT_ID_SWAP_CHAIN_BLIT_VBS,
    DESCLAYOUT_ID_VIEW_GLOBAL,
    DESCLAYOUT_ID_ASSET_INSTANCE,
    DESCLAYOUT_ID_ASSET_VBS,
    DESCLAYOUT_ID_POSONLY_VBS,
    DESCLAYOUT_ID_IMGUI_VBS,
    DESCLAYOUT_ID_IMGUI_TEX,
    DESCLAYOUT_ID_MAX,
};

enum
{
    SHADER_ID_SWAP_CHAIN_BLIT = 0,
    SHADER_ID_IMGUI_DEBUGUI,
    SHADER_ID_Pass1,
    SHADER_ID_Pass2,
    SHADER_ID_MAX,
};
//-----

// Graphics API layer
#define CREATE_RESOURCE(name) ResourceHandle name(const ResourceDesc& resDesc)
CREATE_RESOURCE(CreateResource);

#define DESTROY_RESOURCE(name) void name(ResourceHandle handle)
DESTROY_RESOURCE(DestroyResource);

#define MAP_RESOURCE(name) void* name(ResourceHandle handle)
MAP_RESOURCE(MapResource);

#define UNMAP_RESOURCE(name) void name(ResourceHandle handle)
UNMAP_RESOURCE(UnmapResource);

#define CREATE_DESCRIPTOR(name) DescriptorHandle name(uint32 descLayoutID)
CREATE_DESCRIPTOR(CreateDescriptor);

#define DESTROY_DESCRIPTOR(name) void name(DescriptorHandle handle)
DESTROY_DESCRIPTOR(DestroyDescriptor);

#define DESTROY_ALL_DESCRIPTORS(name) void name()
DESTROY_ALL_DESCRIPTORS(DestroyAllDescriptors);

#define WRITE_DESCRIPTOR(name) void name(uint32 descLayoutID, DescriptorHandle descSetHandle, const DescriptorSetDataHandles* descSetDataHandles)
WRITE_DESCRIPTOR(WriteDescriptor);

#define SUBMIT_CMDS_IMMEDIATE(name) void name(Tk::Graphics::GraphicsCommandStream* graphicsCommandStream)
SUBMIT_CMDS_IMMEDIATE(SubmitCmdsImmediate);

// Graphics command recording
void RecordCommandPushConstant(const uint8* data, uint32 sizeInBytes, uint32 shaderID);
void RecordCommandSetScissor(int32 offsetX, int32 offsetY, uint32 width, uint32 height);
void RecordCommandDrawCall(ResourceHandle indexBufferHandle, uint32 numIndices, uint32 numInstances,
    uint32 vertOffset, uint32 indexOffset, const char* debugLabel, bool immediateSubmit);
void RecordCommandBindShader(uint32 shaderID, uint32 blendState, uint32 depthState, bool immediateSubmit);
void RecordCommandBindDescriptor(uint32 shaderID, const DescriptorHandle descSetHandle, uint32 descSetIndex, bool immediateSubmit);
void RecordCommandMemoryTransfer(uint32 sizeInBytes, ResourceHandle srcBufferHandle, ResourceHandle dstBufferHandle,
    const char* debugLabel, bool immediateSubmit);
void RecordCommandRenderPassBegin(uint32 numColorRTs, const ResourceHandle* colorRTs, ResourceHandle depthRT,
    uint32 renderWidth, uint32 renderHeight, const char* debugLabel, bool immediateSubmit);
void RecordCommandRenderPassEnd(bool immediateSubmit);
void RecordCommandTransitionLayout(ResourceHandle imageHandle, uint32 startLayout, uint32 endLayout,
    const char* debugLabel, bool immediateSubmit);
void RecordCommandClearImage(ResourceHandle imageHandle, 
    const v4f& clearValue, const char* debugLabel, bool immediateSubmit);
void RecordCommandGPUTimestamp(uint32 gpuTimestampID, bool immediateSubmit);
//

// Called only by ShaderManager
#define CREATE_DESCRIPTOR_LAYOUT(name) bool name(uint32 descLayoutID, const DescriptorLayout* descLayout)
CREATE_DESCRIPTOR_LAYOUT(CreateDescriptorLayout);

#define CREATE_GRAPHICS_PIPELINE(name) bool name(void* vertexShaderCode, uint32 numVertexShaderBytes, void* fragmentShaderCode, uint32 numFragmentShaderBytes, uint32 shaderID, uint32 viewportWidth, uint32 viewportHeight, uint32 numColorRTs, const uint32* colorRTFormats, uint32 depthFormat, uint32* descriptorHandles, uint32 numDescriptorHandles)
CREATE_GRAPHICS_PIPELINE(CreateGraphicsPipeline);

#define DESTROY_GRAPHICS_PIPELINE(name) void name(uint32 shaderID)
DESTROY_GRAPHICS_PIPELINE(DestroyGraphicsPipeline);
//

void CreateContext(const Tk::Platform::WindowHandles* windowHandles, uint32 windowWidth, uint32 windowHeight);
void RecreateContext(const Tk::Platform::WindowHandles* windowHandles, uint32 windowWidth, uint32 windowHeight);
void WindowResize();
void WindowMinimized();
void DestroyContext();
void DestroyAllPSOPerms();

bool AcquireFrame();
void ProcessGraphicsCommandStream(const Tk::Graphics::GraphicsCommandStream* graphicsCommandStream, bool immediateSubmit);
void BeginFrameRecording();
void EndFrameRecording();
void SubmitFrameToGPU();

float GetGPUTimestampPeriod();
uint32 GetCurrentFrameInFlightIndex();
void ResolveMostRecentAvailableTimestamps(void* gpuTimestampCPUSideBuffer, uint32 numTimestampsInQuery, bool immediateSubmit);

}
}

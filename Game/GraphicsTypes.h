#pragma once

#include "CoreDefines.h"
#include "Math/VectorTypes.h"
#include "Platform/PlatformGameAPI.h"
#include "Graphics/Common/GraphicsCommon.h"

// Buffer that has no persistent staging buffer
// Meant to updated once with a staging buffer which should
// then be destroyed.
typedef struct static_buffer_data
{
    Tk::Graphics::ResourceHandle gpuBufferHandle;
} StaticBuffer;

typedef struct static_mesh_data
{
    Tk::Graphics::DescriptorHandle m_descriptor;
    StaticBuffer m_positionBuffer;
    StaticBuffer m_uvBuffer;
    StaticBuffer m_normalBuffer;
    StaticBuffer m_indexBuffer;
    uint32 m_numIndices;
} StaticMeshData;

typedef struct mesh_attribute_data
{
    uint32 m_numVertices;
    uint8* m_vertexBufferData_Pos;
    uint8* m_vertexBufferData_UV;
    uint8* m_vertexBufferData_Normal;
    uint8* m_vertexBufferData_Index;
} MeshAttributeData;

typedef struct texture_metadata
{
    v3ui m_dims;
    uint16 m_bitsPerPixel;
    // TODO: file type/ext?
} TextureMetadata;

enum
{
    eRenderPass_MainView,
    eRenderPass_Max
};

struct TransientPrim
{
    Tk::Graphics::ResourceHandle indexBufferHandle;
    Tk::Graphics::ResourceHandle vertexBufferHandle;
    Tk::Graphics::DescriptorHandle descriptor;
    uint32 numVertices;
};

void CreateAnimatedPoly(TransientPrim* prim);
void DestroyAnimatedPoly(TransientPrim* prim);
void UpdateAnimatedPoly(TransientPrim* prim);
void DrawAnimatedPoly(TransientPrim* prim, Tk::Graphics::DescriptorHandle globalData, uint32 shaderID, uint32 blendState, uint32 depthState, Tk::Graphics::GraphicsCommandStream* graphicsCommandStream);

typedef struct game_graphics_data
{
    Tk::Graphics::ResourceHandle m_rtColorHandle;
    Tk::Graphics::ResourceHandle m_rtDepthHandle;

    Tk::Graphics::DescriptorHandle m_DescData_Global;
    Tk::Graphics::ResourceHandle m_DescDataBufferHandle_Global;
    void* m_DescDataBufferMemPtr_Global;

    Tk::Graphics::DescriptorHandle m_swapChainBlitDescHandle;

    TransientPrim m_animatedPolygon;
} GameGraphicsData;

typedef struct descriptor_instance_data
{
    alignas(16) m4f modelMatrix;
} DescriptorData_Instance;

typedef struct descriptor_global_data
{
    alignas(16) m4f viewProj;
} DescriptorData_Global;

template <uint32 numPoints, uint32 numIndices>
struct default_geometry
{
    StaticBuffer m_positionBuffer;
    StaticBuffer m_uvBuffer;
    StaticBuffer m_normalBuffer;
    StaticBuffer m_indexBuffer;
    Tk::Graphics::DescriptorHandle m_descriptor;
    v4f m_points[numPoints];
    v2f m_uvs[numPoints];
    v4f m_normals[numPoints];
    uint32 m_indices[numIndices];
};

// Default geometry
template <uint32 numPoints, uint32 numIndices>
using DefaultGeometry = struct default_geometry<numPoints, numIndices>;

#define DEFAULT_QUAD_NUM_VERTICES 4
#define DEFAULT_QUAD_NUM_INDICES 6
extern DefaultGeometry<DEFAULT_QUAD_NUM_VERTICES, DEFAULT_QUAD_NUM_INDICES> defaultQuad;

void CreateDefaultGeometry(Tk::Graphics::GraphicsCommandStream* graphicsCommandStream);
void DestroyDefaultGeometry();

template <typename DefGeom>
void DestroyDefaultGeometryVertexBufferDescriptor(DefGeom& geom)
{
    Tk::Graphics::DestroyDescriptor(geom.m_descriptor);
    geom.m_descriptor = Tk::Graphics::DefaultDescHandle_Invalid;
}

template <typename DefGeom>
void CreateDefaultGeometryVertexBufferDescriptor(DefGeom& geom)
{
    geom.m_descriptor = Tk::Graphics::CreateDescriptor(Tk::Graphics::DESCLAYOUT_ID_ASSET_VBS);

    Tk::Graphics::DescriptorSetDataHandles descDataHandles[MAX_DESCRIPTOR_SETS_PER_SHADER] = {};
    descDataHandles[0].InitInvalid();
    descDataHandles[0].handles[0] = geom.m_positionBuffer.gpuBufferHandle;
    descDataHandles[0].handles[1] = geom.m_uvBuffer.gpuBufferHandle;
    descDataHandles[0].handles[2] = geom.m_normalBuffer.gpuBufferHandle;
    descDataHandles[1].InitInvalid();
    descDataHandles[2].InitInvalid();

    Tk::Graphics::WriteDescriptor(Tk::Graphics::DESCLAYOUT_ID_ASSET_VBS, geom.m_descriptor, &descDataHandles[0]);
}

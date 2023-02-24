#pragma once

#include "CoreDefines.h"
#include "Math/VectorTypes.h"
#include "Allocators.h"

// TODO: put this somewhere else
#include "Mem.h"
struct Buffer
{
    uint8* m_data = nullptr;
    uint64 m_sizeInBytes = 0;

    void Alloc(uint64 sizeInBytes)
    {
        m_data = (uint8*)Tk::Core::CoreMalloc(sizeInBytes);
        m_sizeInBytes = sizeInBytes;
    }

    void Dealloc()
    {
        Tk::Core::CoreFree(m_data);
        m_data = nullptr;
        m_sizeInBytes = 0;
    }
};

namespace Tk
{
namespace Core
{
namespace Asset
{
// This implementation of OBJ loading is only set up to properly load well-formed OBJ files exported from Autodesk Maya.
// It only loads positions, normals, UVs, and indices.
// It also assumes the mesh has been triangulated already!
// It is generally free to crash or misbehave when loading a nonconforming OBJ file.

struct OBJParseScratchBuffers
{
    Tk::Core::LinearAllocator VertPosAllocator;
    Tk::Core::LinearAllocator VertUVAllocator;
    Tk::Core::LinearAllocator VertNormalAllocator;

    void ResetState()
    {
        VertPosAllocator.ResetState();
        VertUVAllocator.ResetState();
        VertNormalAllocator.ResetState();
    }
};

// Parse the OBJ file and populate existing vertex attribute buffers
TINKER_API void ParseOBJ(Tk::Core::LinearAllocator& PosAllocator, Tk::Core::LinearAllocator& UVAllocator,
    Tk::Core::LinearAllocator& NormalAllocator, Tk::Core::LinearAllocator& IndexAllocator,
    OBJParseScratchBuffers& ScratchBuffers, const uint8* EntireFileBuffer, uint64 FileSize, uint32* OutVertCount);

// Loading of various texture types
#pragma pack(push, 1)
// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
typedef struct bmp_header
{
    uint16 type;
    uint32 fileSizeInBytes;
    int16 reserved1;
    int16 reserved2;
    uint32 offsetToBMPBytes;
} BMPHeader;

typedef struct bmp_info
{
    uint32 structSize;
    int32 width;
    int32 height;
    uint16 planes;
    uint16 bitsPerPixel;
    uint32 compression;
    uint32 sizeInBytes;
    int32 xPPM;
    int32 yPPM;
    uint32 clrUsed;
    uint32 clrImp;
    struct rgb_quad
    {
        uint8 b;
        uint8 g;
        uint8 r;
        uint8 reserved;
    };
} BMPInfo;
#pragma pack(pop)

TINKER_API BMPInfo GetBMPInfo(uint8* entireFileBuffer);

TINKER_API void SaveBMP(Buffer* outputBuffer, uint8* inputData, uint32 width, uint32 height, uint16 bitsPerPx);

}
}
}

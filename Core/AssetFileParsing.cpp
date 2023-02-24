#include "AssetFileParsing.h"
#include "Mem.h"

#include <string.h>

namespace Tk
{
namespace Core
{
namespace Asset
{

static const char LineEndingChars[2] = { '\n', '\r' };
static const char EOFChar = '\0';
static const char WhiteSpaceChars[2] = { ' ', '/' };
#define MAX_SCRATCH_WORD_LEN 32

static bool HitSpecialChar(char TheChar, const char* SpecialCharList, uint32 NumCharsToCheck)
{
    for (uint32 i = 0; i < NumCharsToCheck; ++i)
    {
        if (TheChar == SpecialCharList[i])
            return true;
    }
    return false;
}

static void scanLine(const uint8* buffer, uint64* currentIndex)
{
    // Scan up to the line ending
    while (!HitSpecialChar(buffer[*currentIndex], LineEndingChars, ARRAYCOUNT(LineEndingChars)))
    {
        if (HitSpecialChar(buffer[*currentIndex], &EOFChar, 1))
            return; // marks EOF

        ++*currentIndex;
    }

    // Scan to the first character after the line ending
    while (HitSpecialChar(buffer[*currentIndex], LineEndingChars, ARRAYCOUNT(LineEndingChars)))
    {
        if (HitSpecialChar(buffer[*currentIndex], &EOFChar, 1))
            return; // marks EOF

        ++* currentIndex;
    }
}

static void scanWord(const uint8* buffer, uint64* currentIndex)
{
    while (!HitSpecialChar(buffer[*currentIndex], LineEndingChars, ARRAYCOUNT(LineEndingChars)) &&
           !HitSpecialChar(buffer[*currentIndex], WhiteSpaceChars, ARRAYCOUNT(WhiteSpaceChars)))
    {
        ++*currentIndex;
    }
}

static void scanWhiteSpace(const uint8* buffer, uint64* currentIndex)
{
    while (HitSpecialChar(buffer[*currentIndex], WhiteSpaceChars, ARRAYCOUNT(WhiteSpaceChars)))
    {
        ++*currentIndex;
    }
}

static void scanWordIntoBuffer(const uint8* buffer, uint64* currentIndex, char* NextWord, uint32 NextWordMaxLen)
{
    // Scan to the next word separated by white space
    scanWhiteSpace(buffer, currentIndex);

    // We are now on the first character of the word
    uint64 wordStartIndex = *currentIndex;

    // Scan to the end of the word
    scanWord(buffer, currentIndex);

    uint32 numBytesToCopy = (uint32)(*currentIndex - wordStartIndex);
    numBytesToCopy = Min(NextWordMaxLen, numBytesToCopy);
    memcpy(NextWord, buffer + wordStartIndex, numBytesToCopy);
}

static void ReadWordsIntoVertBuffer(uint32 NumWords, float* OutVertexData, const uint8* EntireFileBuffer, uint64* currentIndex)
{
    for (uint32 uiWord = 0; uiWord < NumWords; ++uiWord)
    {
        char NextWord[MAX_SCRATCH_WORD_LEN];
        memset(NextWord, 0, ARRAYCOUNT(NextWord) * sizeof(char));
        scanWordIntoBuffer(EntireFileBuffer, currentIndex, NextWord, ARRAYCOUNT(NextWord));
        float WordAsFloat = (float)atof(NextWord);
        OutVertexData[uiWord] = WordAsFloat;
    }
}

void ParseOBJ(Tk::Core::LinearAllocator& PosAllocator, Tk::Core::LinearAllocator& UVAllocator,
    Tk::Core::LinearAllocator& NormalAllocator, Tk::Core::LinearAllocator& IndexAllocator,
    OBJParseScratchBuffers& ScratchBuffers, const uint8* EntireFileBuffer, uint64 FileSize, uint32* OutVertCount)
{
    // Counter gets advanced as we scan lines of the files
    uint64 currentIndex = 0;

    // Keep a counter for number of vertices
    *OutVertCount = 0;
    
    // Need global counter for indices
    uint32 indicesCounter = 0;

    // Scan until we hit the null terminator which is used here to mark EOF, and for safety, stay within file size
    while (currentIndex < FileSize && !HitSpecialChar(EntireFileBuffer[currentIndex], &EOFChar, 1))
    {
        if (EntireFileBuffer[currentIndex] == 'v' && EntireFileBuffer[currentIndex + 1] == ' ') // Vertex positions
        {
            // skip the 'v'
            scanWord(EntireFileBuffer, &currentIndex);

            v4f* VertBufferPtr = (v4f*)ScratchBuffers.VertPosAllocator.Alloc(sizeof(v4f), 1);
            const uint32 numWordsPerVert = 3;
            ReadWordsIntoVertBuffer(numWordsPerVert, (float*)VertBufferPtr, EntireFileBuffer, &currentIndex);
            (*VertBufferPtr)[numWordsPerVert] = 1.0f; // set homogeneous coord to 1
        }
        else if (EntireFileBuffer[currentIndex] == 'v' && EntireFileBuffer[currentIndex + 1] == 't') // Vertex texture coordinates
        {
            // skip the 'vt'
            scanWord(EntireFileBuffer, &currentIndex);

            v2f* VertBufferPtr = (v2f*)ScratchBuffers.VertUVAllocator.Alloc(sizeof(v2f), 1);
            const uint32 numWordsPerVert = 2;
            ReadWordsIntoVertBuffer(numWordsPerVert, (float*)VertBufferPtr, EntireFileBuffer, &currentIndex);
        }
        else if (EntireFileBuffer[currentIndex] == 'v' && EntireFileBuffer[currentIndex + 1] == 'n') // Vertex normals
        {
            // skip the 'vn'
            scanWord(EntireFileBuffer, &currentIndex);

            v4f* VertBufferPtr = (v4f*)ScratchBuffers.VertNormalAllocator.Alloc(sizeof(v4f), 1);
            const uint32 numWordsPerVert = 3;
            ReadWordsIntoVertBuffer(numWordsPerVert, (float*)VertBufferPtr, EntireFileBuffer, &currentIndex);
            VertBufferPtr->w = 0.0f;
        }
        else if (EntireFileBuffer[currentIndex] == 'f' && EntireFileBuffer[currentIndex + 1] == ' ') // Indices
        {
            // skip the 'f'
            scanWord(EntireFileBuffer, &currentIndex);

            const uint8 numWordsPerFace = 3;
            for (uint8 uiWord = 0; uiWord < numWordsPerFace; ++uiWord)
            {
                const uint8 numIndicesPerFace = 3;
                uint32 newIndices[numIndicesPerFace] = {};

                // NOTE: '/' characters are treated as white space when scanning chars
                char NextWord[MAX_SCRATCH_WORD_LEN];

                // Normalize indices to start from 0, OBJ convention is to start from 1
                memset(NextWord, 0, ARRAYCOUNT(NextWord) * sizeof(char));
                scanWordIntoBuffer(EntireFileBuffer, &currentIndex, NextWord, ARRAYCOUNT(NextWord));
                newIndices[0] = (uint32)atoi(NextWord) - 1;

                memset(NextWord, 0, ARRAYCOUNT(NextWord) * sizeof(char));
                scanWordIntoBuffer(EntireFileBuffer, &currentIndex, NextWord, ARRAYCOUNT(NextWord));
                newIndices[1] = (uint32)atoi(NextWord) - 1;

                memset(NextWord, 0, ARRAYCOUNT(NextWord) * sizeof(char));
                scanWordIntoBuffer(EntireFileBuffer, &currentIndex, NextWord, ARRAYCOUNT(NextWord));
                newIndices[2] = (uint32)atoi(NextWord) - 1;

                v4f*    FinalVertPosBufferPtr    = (v4f*)PosAllocator.Alloc(sizeof(v4f), 1);
                v2f*    FinalVertUVBufferPtr     = (v2f*)UVAllocator.Alloc(sizeof(v2f), 1);
                v4f*    FinalVertNormalBufferPtr = (v4f*)NormalAllocator.Alloc(sizeof(v4f), 1);
                uint32* FinalVertIndexBufferPtr  = (uint32*)IndexAllocator.Alloc(sizeof(uint32), 1);

                *FinalVertPosBufferPtr = ((v4f*)ScratchBuffers.VertPosAllocator.m_ownedMemPtr)[newIndices[0]];
                *FinalVertUVBufferPtr = ((v2f*)ScratchBuffers.VertUVAllocator.m_ownedMemPtr)[newIndices[1]];
                *FinalVertNormalBufferPtr = ((v4f*)ScratchBuffers.VertNormalAllocator.m_ownedMemPtr)[newIndices[2]];
                *FinalVertIndexBufferPtr = indicesCounter;

                // TODO: no vertex deduplication currently happens

                ++indicesCounter;
            }
        }
        else
        {
            // Proceed to next line
            scanLine(EntireFileBuffer, &currentIndex);
        }
    }

    *OutVertCount = indicesCounter;
}

BMPInfo GetBMPInfo(uint8* entireFileBuffer)
{
    // NOTE: assumes the buffer is a well-formed bmp file
    // Skips the file header, which is immediately followed by the bmp info
    return *(BMPInfo*)(entireFileBuffer + sizeof(BMPHeader));
}

void SaveBMP(Buffer* outputBuffer, uint8* inputData, uint32 width, uint32 height, uint16 bitsPerPx)
{
    TINKER_ASSERT(outputBuffer);

    uint32 imgSize = width * height * bitsPerPx/8;

    BMPHeader header;
    memset(&header, 0, sizeof(BMPHeader));
    header.type = 0x4D42;
    header.fileSizeInBytes = imgSize;
    header.offsetToBMPBytes = sizeof(BMPHeader) + sizeof(BMPInfo);

    outputBuffer->Alloc(imgSize + header.offsetToBMPBytes);
    uint8* data = outputBuffer->m_data;

    BMPInfo info;
    memset(&info, 0, sizeof(BMPInfo));
    info.structSize = sizeof(BMPInfo);
    info.width = width;
    info.height = height;
    info.planes = 1;
    info.bitsPerPixel = bitsPerPx;
    info.compression = 0;
    info.sizeInBytes = imgSize;
    info.xPPM = 0;
    info.yPPM = 0;
    info.clrUsed = 0;
    info.clrImp = 0;

    // TODO: simd stream
    memcpy(data, &header, sizeof(header));
    data += sizeof(header);
    memcpy(data, &info, sizeof(info));
    data += sizeof(info);
    memcpy(data, inputData, imgSize);
}

}
}
}

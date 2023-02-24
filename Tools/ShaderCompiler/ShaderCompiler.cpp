#include <atlbase.h>
#include <inc/dxcapi.h>

#include <stdio.h>

#include "ShaderCompiler.h"
#include "DataStructures/Vector.h"
#include "Platform/PlatformGameAPI.h"
#include "StringTypes.h"

namespace Tk
{
namespace ShaderCompiler
{

static CComPtr<IDxcUtils> g_pUtils;
static CComPtr<IDxcCompiler3> g_pCompiler;
static CComPtr<IDxcIncludeHandler> g_pIncludeHandler;
static uint32 isInitted = 0;

Tk::Core::Vector<const wchar_t*> g_args;

#ifdef _SHADERS_SRC_DIR
#define SHADERS_SRC_DIR STRINGIFY(_SHADERS_SRC_DIR)
#endif

#ifdef _SHADERS_SPV_DIR
#define SHADERS_SPV_DIR STRINGIFY(_SHADERS_SPV_DIR)
#endif

namespace ShaderType
{

enum : uint32
{
    Vertex = 0,
    Pixel,
    Compute,
    Max
};
}

static const char* ShaderFileSuffixRegexs[ShaderType::Max] =
{
    SHADERS_SRC_DIR "*_VS.hlsl",
    SHADERS_SRC_DIR "*_PS.hlsl",
    SHADERS_SRC_DIR "*_CS.hlsl",
};

// TODO: shader #defines

#define ENTRY_POINT_NAME_WCHAR L"main"

static const wchar_t* CompileFlags_Common[] =
{
    L"DXC_ARG_WARNINGS_ARE_ERRORS",
    L"DXC_ARG_DEBUG",
    L"DXC_ARG_PACK_MATRIX_COLUMN_MAJOR",
    L"-E ",
    ENTRY_POINT_NAME_WCHAR,
};

static const wchar_t* CompileFlags_VkSpecific[] =
{
    L"-spirv",
    L"-fspv-target-env=vulkan1.3",
};

static const wchar_t* CompileFlags_Debug[] =
{
    L"-Zi",
    //L"-Qstrip_debug",
    L"-Zss",
};

static const wchar_t* CompileFlags_ShaderSpecific[ShaderType::Max][1] =
{
    { L"-T vs_6_7" },
    { L"-T ps_6_7" },
    { L"-T cs_6_7" },
};

static void AppendArgs_(const wchar_t** argsToAppend, uint32 numArgsToAppend, Tk::Core::Vector<const wchar_t*>& argsOut)
{
    for (uint32 i = 0; i < numArgsToAppend; ++i)
    {
        argsOut.PushBackRaw(argsToAppend[i]);
    }
}
#define AppendArgsToList(args, argsList) AppendArgs_(args, ARRAYCOUNT(args), argsList);

static uint32 CompileFile(CComPtr<IDxcCompiler3> pCompiler, CComPtr<IDxcUtils> pUtils, CComPtr<IDxcIncludeHandler> pIncludeHandler, const wchar_t* args, uint32 numArgs, const wchar_t* shaderFilepath, const wchar_t* shaderFilenameWithExt)
{
    CComPtr<IDxcBlobEncoding> pSource = nullptr;
    pUtils->LoadFile((LPCWSTR)shaderFilepath, nullptr, &pSource);
    DxcBuffer Source;
    Source.Ptr = pSource->GetBufferPointer();
    Source.Size = pSource->GetBufferSize();
    Source.Encoding = DXC_CP_ACP;

    CComPtr<IDxcResult> pResults;
    HRESULT compileStatus = pCompiler->Compile(&Source, (LPCWSTR*)args, numArgs, pIncludeHandler, IID_PPV_ARGS(&pResults));
    if (FAILED(compileStatus))
    {
        // Something bad happened inside DXC
        TINKER_ASSERT(0);
        return ErrCode::HasErrors;
    }

    uint32 errCode = ErrCode::HasErrors;

    CComPtr<IDxcBlobUtf8> pErrors = nullptr;
    pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);

    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
    {
        printf("Warnings and Errors:\n%s\n", pErrors->GetStringPointer());
        return ErrCode::HasErrors;
    }
    
    // Compilation succeeded
    errCode = ErrCode::Success;
    // TODO: figure out how to determine if the shader has only warnings

    // Get shader hash
    CComPtr<IDxcBlob> pHash = nullptr;
    if (SUCCEEDED(pResults->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHash), nullptr)) && pHash != nullptr)
    {
        DxcShaderHash* pHashBuf = (DxcShaderHash*)pHash->GetBufferPointer();
        // TODO: do something with it
    }

    CComPtr<IDxcBlob> pShader = nullptr;
    if (SUCCEEDED(pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), nullptr)) && pShader != nullptr)
    {
        // For now, just write out the spv file
        static const uint32 filepathMax = 2048;
        Tk::Core::StrFixedBuffer<2048> shaderFilepathSpv;
        shaderFilepathSpv.Clear();
        shaderFilepathSpv.Append(SHADERS_SPV_DIR);
        char* shaderFilenameSpvStart = shaderFilepathSpv.EndOfStrPtr();

        // Remove .hlsl ext, replace with .spv
        static const char* hlslExt = "hlsl";
        const uint32 hlslExtLen = (uint32)strlen(hlslExt);
        uint32 shaderFilenameNoExtLen = (uint32)wcslen(shaderFilenameWithExt) - hlslExtLen;
        shaderFilepathSpv.AppendWChar(shaderFilenameWithExt, shaderFilenameNoExtLen);

        // Complete string with .spv extension and null terminator
        shaderFilepathSpv.Append("spv");
        shaderFilepathSpv.NullTerminate();
        uint32 fileErr = Tk::Platform::WriteEntireFile(shaderFilepathSpv.m_data, (uint32)pShader->GetBufferSize(), (uint8*)pShader->GetBufferPointer());
        if (!fileErr)
        {
            printf("Wrote: %s\n", shaderFilepathSpv.m_data);
        }
        else
        {
            errCode = ErrCode::NonShaderError;
            printf("Error writing spv file: %s\n", shaderFilepathSpv.m_data);
        }
    }

    return errCode;
}

uint32 Init()
{
    g_args.Reserve(32);
    if (!(FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&g_pUtils))) ||
          FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&g_pCompiler))) ||
          FAILED(g_pUtils->CreateDefaultIncludeHandler(&g_pIncludeHandler))))
    {
        isInitted = 1;
        return ErrCode::Success;
    }
    else
    {
        isInitted = 0;
        return ErrCode::NonShaderError;
    }
}

uint32 CompileAllShadersDX()
{
    printf("DX codepath not implemented yet :)");
    return ErrCode::NonShaderError;
}

uint32 CompileAllShadersVK()
{
    if (!isInitted)
        return ErrCode::NonShaderError;
    
    // Start compilation
    uint32 errorCode = ErrCode::Success;

    // NOTE: only need this due to char -> w_char games
    wchar_t currShaderFilepath[2048] = {};
    uint32 filenameMax = ARRAYCOUNT(currShaderFilepath);
    size_t numCharsWritten = 0;
    mbstowcs_s(&numCharsWritten, currShaderFilepath, ARRAYCOUNT(currShaderFilepath), SHADERS_SRC_DIR, strlen(SHADERS_SRC_DIR));
    --numCharsWritten; // We are going to overwrite the null terminator
    uint32 numCharsRemaining = filenameMax - (uint32)numCharsWritten;
    wchar_t* shaderFilenameStart = &currShaderFilepath[numCharsWritten];

    for (uint32 uiShaderType = 0; uiShaderType < ShaderType::Max; ++uiShaderType)
    {
        g_args.Clear();
        AppendArgsToList(CompileFlags_Common, g_args);
        AppendArgsToList(CompileFlags_VkSpecific, g_args);
        AppendArgsToList(CompileFlags_ShaderSpecific[uiShaderType], g_args);

        Tk::Platform::FileHandle findFileHandle = Tk::Platform::FindFileOpen(ShaderFileSuffixRegexs[uiShaderType], shaderFilenameStart, numCharsRemaining);
        uint32 findFileError = findFileHandle.h == findFileHandle.eInvalidValue;
        while (!findFileError)
        {
            printf("\nCompiling: %ls...\n", shaderFilenameStart);

            uint32 compileError = CompileFile(g_pCompiler, g_pUtils, g_pIncludeHandler, (const wchar_t*)g_args.Data(), g_args.Size(), currShaderFilepath, shaderFilenameStart);
            if (compileError == ErrCode::Success || compileError == ErrCode::HasWarnings)
            {
                // TODO: add entry to hashmap for material library, get the bytecode from the compile call
            }
            else
            {
                errorCode = compileError;
            }

            // Reset shader name but keep base path
            memset((uint8*)shaderFilenameStart, 0, numCharsRemaining * sizeof(wchar_t));
            findFileError = Tk::Platform::FindFileNext(findFileHandle, shaderFilenameStart, numCharsRemaining);
        }
        Tk::Platform::FindFileClose(findFileHandle);
    }

    return errorCode;
}

}
}

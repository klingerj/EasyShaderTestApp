#include "ShaderCompiler.h"

#include <stdio.h>
#include <cstring>

#include "MurmurHash3.h"

int main(int argc, char* argv[])
{
    uint32 bVulkan = false;
    if (argc != 2)
    {
        printf("Invalid number of arguments provided. Usage: TinkerSC.exe [VK | DX]\n");
        return Tk::ShaderCompiler::ErrCode::NonShaderError;
    }
    else
    {
        if (strncmp(argv[1], "VK", strlen("VK")) == 0)
        {
            bVulkan = 1u;
        }
        else if (strncmp(argv[1], "DX", strlen("DX")) == 0)
        {
            bVulkan = 0u;
        }
        else
        {
            printf("Unrecognized argument provided. Usage: TinkerSC.exe [VK | DX]\n");
            return Tk::ShaderCompiler::ErrCode::NonShaderError;
        }
    }

    Tk::ShaderCompiler::Init();

    uint32 result = Tk::ShaderCompiler::ErrCode::NonShaderError;
    if (bVulkan)
        result = Tk::ShaderCompiler::CompileAllShadersVK();
    else
        result = Tk::ShaderCompiler::CompileAllShadersDX();

    switch (result)
    {
    case Tk::ShaderCompiler::ErrCode::Success:
    case Tk::ShaderCompiler::ErrCode::HasWarnings:
    {
        printf("Compilation succeeded.\n");
        break;
    }

    case Tk::ShaderCompiler::ErrCode::HasErrors:
    {
        printf("Compilation failed with shader errors.\n");
        break;
    }

    default:
    {
        printf("Compilation failed due to some error.\n");
        break;
    }
    }

    return result;
}

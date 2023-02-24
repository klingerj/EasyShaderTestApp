#pragma once

#include "CoreDefines.h"

namespace Tk
{
namespace Graphics
{
namespace ShaderManager
{
    void Startup();
    void Shutdown();

    void LoadAllShaders(uint32 windowWidth, uint32 windowHeight);
    void LoadAllShaderResources(uint32 windowWidth, uint32 windowHeight);
    void CreateWindowDependentResources(uint32 newWindowWidth, uint32 newWindowHeight);
    void ReloadShaders(uint32 newWindowWidth, uint32 newWindowHeight);
}
}
}

#pragma once

namespace Tk
{
namespace Graphics
{
    struct GraphicsCommandStream;
    struct ResourceHandle;
}
}

namespace DebugUI
{
    void Init(Tk::Graphics::GraphicsCommandStream* graphicsCommandStream);
    void Shutdown();
    void NewFrame();
    void Render(Tk::Graphics::GraphicsCommandStream* graphicsCommandStream, Tk::Graphics::ResourceHandle renderTarget);
    void ToggleEnable();

    void UI_RenderPassStats();
}

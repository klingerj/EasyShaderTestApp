# Tinker Engine

My second game engine project. Made with things learned from Handmade Hero, coworkers, and lessons learned from the older [Joe Engine](https://github.com/klingerj/Joe-Engine). Nearly everything written from scratch, extremely minimal libraries/dependencies. Tinker has superior software architecture, minimal dependencies, compiles faster, and has other debatably cooler features and capabilities than the Joe Engine.

Currently, the project renders a few instanced models with a Z prepass (fairly barebones graphics-wise):  
![](Output/TestImages/gameScreenshot.png)

Output from the SPIR-V virtual machine, evaluating a very simple Lambertian shader:  
![](Output/TestImages/spirv_output.bmp)

<!--- Early in the project, I added very simple software raytracing to try to match the hardware rasterized output of the game: 
Raytraced | Rasterized  
:--------:|:----------:  
![](Output/TestImages/raytraceOutput.bmp) | ![](Output/TestImages/rasterRef.bmp) --->

### Platforms supported currently:
* Windows 10 / x86

### Graphics API backends supported currently:
* Vulkan

### Project hierarchy description
* <code>Assets/</code> - art files to load
* <code>Benchmark/</code> - simple setup for running benchmarked code
* <code>Build/</code> - dir for exe/libs/dlls (generated)
* <code>Core/</code> - core/useful engine code
  * <code>DataStructures/</code> - data structures, e.g. vector and hashmap
  * <code>Math/</code> - math ops, e.g. matrix multiply (SIMD-accelerated)
  * <code>Platform/</code> - platform/threading api layer as well as app main, e.g. Windows
  * <code>Raytracing/</code> - minimal/naive raytracing and octree code (WIP)
  * <code>Utility/</code> - logging, mem alloc tracking, code block timing
* <code>DebugUI/</code> - debug ui / imgui layer
* <code>Game/</code> - game code, builds as dll
* <code>Graphics/</code> - graphics api layer as well as backends, e.g. Vulkan
* <code>Output/</code> - dumping ground for output files, e.g. raytraced image
* <code>Scripts/</code> - build scripts and some helpful project scripts
* <code>Shaders/</code> - all gpu shader code
  * <code>hlsl/</code> - shader source
  * <code>spv/</code> - compiled shader bytecode for vulkan backend
* <code>SPIR-V-VM/</code> - virtual machine for evaluating SPIR-V shaders (WIP)
* <code>Test/</code> - simple setup for running unit tests
* <code>ThirdParty/</code> - external libraries
  * <code>dxc_2022_07_18/</code> - dxc release lib
  * <code>imgui-docking/</code> - Imgui lib, docking branch
  * <code>MurmurHash3/</code> - Murmur3 run-time and compile-time string hashing - some modifications by me
* <code>Tools/</code> - code for separate tools
  * <code>ShaderCompiler/</code> - code that calls into DXC to compile HLSL shaders to SPIR-V (+DXIL eventually) 
* <code>ToolsBin/</code> - built binaries of code from <code>Tools/</code>
* <code>Utils/</code> - random stuff, e.g. Natvis files

### List of features implemented currently:
* Platform layer
  * Builds as the exe
  * Win32 backend support 
  * Automatically hotloads newly built game code dll
  * Simple thread-job system with SPSC lockless queue
* Graphics
  * API-agnostic graphics layer, compiles with desired backend
    * Vulkan backend support
      * Use of dynamic rendering API feature
    * Render pass timing with GPU timestamps
  * Builds with game dll, meaning the graphics system is hotloadable
  * Draw call batching
  * Instanced rendering
  * Shader hotloading on keypress
    * Calls into DXC lib
  * Shader pipeline state objects handled as blend/depth state permutations
* Custom data structures
  * Vector
  * Hash map
  * Ring buffer
* Simple linear algebra library
* Custom memory allocators
* Memory allocation tracking
* Compile-time and run-time string hashing using Murmur
* Asset file loading
  * OBJ models, BMP textures
* ImGui debug UI
* Simple unit testing and benchmarking frameworks
* WIP features
  * SPIR-V virtual machine
    * tested to evaluate only some very simple SPIR-V shaders
    * written in C99 and builds separately from rest of Tinker project
  * Simple CPU-side raytracing (WIP)

### Roadmap of future features:
* Compute shader support
* Post processing
* Asset file cooking
* DX12 graphics backend
* Asset streaming

### Build instructions
You will need to have installed:
* Visual Studio 2019, because the project will currently check for  
<code>C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat</code>  
(<code>Community</code> and <code>Enterprise</code> versions will also work)
* Vulkan SDK version 1.3.216.0 or higher from [LunarG](https://vulkan.lunarg.com/sdk/home#windows)

To Build:
* Run <code>proj_shell.bat</code> in the <code>Scripts/</code> directory. You can do this by double-clicking in the file explorer or from command line.
* In that shell window, run <code>ez-build_release.bat</code>. This will build every Tinker project.
You should now be able to run the game via <code>run_game.bat</code>.  
You can find more detailed info on the various build projects [here](Scripts/README.md).

### Dependencies
* [DirectXShaderCompiler (DXC)](https://github.com/microsoft/DirectXShaderCompiler)
* [Murmur Hash 3, MIT license](https://github.com/aappleby/smhasher)
  * Also referenced [this gist](https://gist.github.com/oteguro/10538695) when implementing compile-time hashing with murmur
* [Imgui - docking branch, MIT license](https://github.com/ocornut/imgui)

### Assets used:  
* [CGTrader - Fire Elemental by inalaatzu](https://www.cgtrader.com/free-3d-models/character/fantasy/fire-elemental-29c02a51-2d44-4c4b-9e73-fc5899cd690d)  
* [CGTrader - RTX 3090 by bemtevi3d](https://www.cgtrader.com/free-3d-models/electronics/computer/rtx-3090-graphic-card-3d-model)

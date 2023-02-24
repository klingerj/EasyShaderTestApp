#pragma once

#include "CoreDefines.h"
#include "PlatformGameThreadAPI.h"

struct ImGuiContext;

namespace Tk
{

namespace Graphics
{
struct GraphicsCommandStream;
}

namespace Platform
{

// TODO: tighten this up for other platforms, probably roll it into a window creation function
struct WindowHandles
{
    uint64 procInstHandle;
    uint64 windowInstHandle;
};

struct FileHandle
{
    unsigned long long h;
    enum
    {
        eInvalidValue = (uint64)-1,
    };
};

#define GET_PLATFORM_WINDOW_HANDLES(name) TINKER_API WindowHandles* name()
GET_PLATFORM_WINDOW_HANDLES(GetPlatformWindowHandles);

#define PRINT_DEBUG_STRING(name) TINKER_API void name(const char* str)
PRINT_DEBUG_STRING(PrintDebugString);

#define ALLOC_ALIGNED_RAW(name) TINKER_API void* name(size_t size, size_t alignment)
ALLOC_ALIGNED_RAW(AllocAlignedRaw);

#define FREE_ALIGNED_RAW(name) TINKER_API void name(void* ptr)
FREE_ALIGNED_RAW(FreeAlignedRaw);

#define READ_ENTIRE_FILE(name) TINKER_API uint32 name(const char* filename, uint32 fileSizeInBytes, uint8* buffer)
READ_ENTIRE_FILE(ReadEntireFile);

#define WRITE_ENTIRE_FILE(name) TINKER_API uint32 name(const char* filename, uint32 fileSizeInBytes, uint8* buffer)
WRITE_ENTIRE_FILE(WriteEntireFile);

#define GET_ENTIRE_FILE_SIZE(name) TINKER_API uint32 name(const char* filename)
GET_ENTIRE_FILE_SIZE(GetEntireFileSize);

#define FIND_FILE_OPEN(name) TINKER_API FileHandle name(const char* dirWithFileExts, wchar_t* outFilename, uint32 outFilenameMax)
FIND_FILE_OPEN(FindFileOpen);

#define FIND_FILE_NEXT(name) TINKER_API uint32 name(FileHandle prevFindFileHandle, wchar_t* outFilename, uint32 outFilenameMax)
FIND_FILE_NEXT(FindFileNext);

#define FIND_FILE_CLOSE(name) TINKER_API void name(FileHandle handle)
FIND_FILE_CLOSE(FindFileClose);

#define INIT_NETWORK_CONNECTION(name) TINKER_API int name()
INIT_NETWORK_CONNECTION(InitNetworkConnection);

#define END_NETWORK_CONNECTION(name) TINKER_API int name()
END_NETWORK_CONNECTION(EndNetworkConnection);

#define SEND_MESSAGE_TO_SERVER(name) TINKER_API int name()
SEND_MESSAGE_TO_SERVER(SendMessageToServer);

// ImGui to platform (Win32, etc) functions
// TODO This is just going to be "technical debt" for now, we need a better defined platform layer for the game dll probably

// These must match imgui.h
typedef void*   (*ImGuiMemAllocFuncWrapper)(size_t sz, void* user_data);
typedef void    (*ImGuiMemFreeFuncWrapper)(void* ptr, void* user_data);
#define IMGUI_CREATE(name) TINKER_API void name(ImGuiContext* context, ImGuiMemAllocFuncWrapper mallocWrapper, ImGuiMemFreeFuncWrapper freeWrapper)
IMGUI_CREATE(ImguiCreate);

#define IMGUI_NEW_FRAME(name) TINKER_API void name()
IMGUI_NEW_FRAME(ImguiNewFrame);

#define IMGUI_DESTROY(name) TINKER_API void name()
IMGUI_DESTROY(ImguiDestroy);
//

namespace Keycode
{
enum : uint32
{
    eA = 0,
    eB,
    eC,
    eD,
    eE,
    eF,
    eG,
    eH,
    eI,
    eJ,
    eK,
    eL,
    eM,
    eN,
    eO,
    eP,
    eQ,
    eR,
    eS,
    eT,
    eU,
    eV,
    eW,
    eX,
    eY,
    eZ,
    e0,
    e1,
    e2,
    e3,
    e4,
    e5,
    e6,
    e7,
    e8,
    e9,
    eF1,
    eF2,
    eF3,
    eF4,
    eF5,
    eF6,
    eF7,
    eF8,
    eF9,
    eF10,
    eF11,
    eF12,
    eMax
};
}

namespace Mousecode
{
enum : uint32
{
    eMouseMoveVertical = 0,
    eMouseMoveHorizontal,
    // TODO: other mouse controls, e.g. click
    eMax
};
}

typedef struct keycode_state
{
    uint8 isDown = false; // is the key currently down
    uint8 numStateChanges = 0; // number of times the key went up/down
} KeycodeState;

typedef struct mousecode_state
{
    union
    {
        int32 displacement; // in pixels, for mouse movement
        // TODO: add mouse click state data, probably similar to keycode state data
        // - maybe refactor that into a more general button state, we will probably use it for game pad input too
    };
} MousecodeState;

struct InputStateDeltas
{
    // TODO: gamepad input

    // Keyboard input
    KeycodeState keyCodes[Keycode::eMax];

    // Mouse input
    MousecodeState mouseCodes[Mousecode::eMax];
};

// Game side
#define GAME_UPDATE(name) uint32 name(uint32 windowWidth, uint32 windowHeight, const Tk::Platform::InputStateDeltas* inputStateDeltas)
typedef GAME_UPDATE(game_update);

#define GAME_DESTROY(name) void name()
typedef GAME_DESTROY(game_destroy);

#define GAME_WINDOW_RESIZE(name) void name(uint32 newWindowWidth, uint32 newWindowHeight)
typedef GAME_WINDOW_RESIZE(game_window_resize);

}
}

#pragma once

#include "Platform/PlatformGameAPI.h"

#define INPUT_CALLBACK(name) void name(uint32 param)
typedef INPUT_CALLBACK(input_callback_func);

struct InputManager
{
private:
    typedef struct input_state
    {
        Tk::Platform::KeycodeState keyCodes[Tk::Platform::Keycode::eMax];
        Tk::Platform::MousecodeState mouseCodes[Tk::Platform::Mousecode::eMax];
    } InputState;
    InputState m_currentInputState = {};
    InputState m_previousInputState = {};
    
    input_callback_func* m_callbacks_keyDown[Tk::Platform::Keycode::eMax] = {};
    input_callback_func* m_callbacks_keyDownRepeat[Tk::Platform::Keycode::eMax] = {};
    // TODO: key up callbacks
    input_callback_func* m_callbacks_mouse[Tk::Platform::Mousecode::eMax] = {};

public:
    void BindKeycodeCallback_KeyDown(uint32 keycode, input_callback_func callback);
    void BindKeycodeCallback_KeyDownRepeat(uint32 keycode, input_callback_func callback);
    void BindMousecodeCallback(uint32 mousecode, input_callback_func callback);
    void UpdateAndDoCallbacks(const Tk::Platform::InputStateDeltas* inputStateDeltas);
};

extern InputManager g_InputManager;

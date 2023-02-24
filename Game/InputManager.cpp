#include "InputManager.h"

using namespace Tk;

InputManager g_InputManager = {};

void InputManager::BindKeycodeCallback_KeyDown(uint32 keycode, input_callback_func callback)
{
    TINKER_ASSERT(keycode < Tk::Platform::Keycode::eMax); // valid keycode
    TINKER_ASSERT(callback); // non-null function pointer
    m_callbacks_keyDown[keycode] = callback;
}

void InputManager::BindKeycodeCallback_KeyDownRepeat(uint32 keycode, input_callback_func callback)
{
    TINKER_ASSERT(keycode < Tk::Platform::Keycode::eMax); // valid keycode
    TINKER_ASSERT(callback); // non-null function pointer
    m_callbacks_keyDownRepeat[keycode] = callback;
}

void InputManager::BindMousecodeCallback(uint32 mousecode, input_callback_func callback)
{
    TINKER_ASSERT(mousecode < Tk::Platform::Mousecode::eMax); // valid keycode
    TINKER_ASSERT(callback); // non-null function pointer
    m_callbacks_mouse[mousecode] = callback;
}

void InputManager::UpdateAndDoCallbacks(const Platform::InputStateDeltas* inputStateDeltas)
{
    m_previousInputState = m_currentInputState;

    // Apply deltas to input state

    for (uint32 uiKeycode = 0; uiKeycode < Platform::Keycode::eMax; ++uiKeycode)
    {
        if (inputStateDeltas->keyCodes[uiKeycode].numStateChanges > 0)
        {
            m_currentInputState.keyCodes[uiKeycode].isDown = inputStateDeltas->keyCodes[uiKeycode].isDown;
            m_currentInputState.keyCodes[uiKeycode].numStateChanges += inputStateDeltas->keyCodes[uiKeycode].numStateChanges;
            // TODO: reset number of state changes at some point, every second or every several frames or something?
        }
    }

    for (uint32 uiMousecode = 0; uiMousecode < Platform::Mousecode::eMax; ++uiMousecode)
    {
        m_currentInputState.mouseCodes[uiMousecode].displacement = inputStateDeltas->mouseCodes[uiMousecode].displacement;
    }

    // Process current state

    for (uint32 uiKeycode = 0; uiKeycode < Platform::Keycode::eMax; ++uiKeycode)
    {
        // Initial downpress
        if (m_currentInputState.keyCodes[uiKeycode].isDown && !m_previousInputState.keyCodes[uiKeycode].isDown)
        {
            input_callback_func* cbFunc = m_callbacks_keyDown[uiKeycode];
            if (cbFunc)
            {
                cbFunc(1); // TODO: figure out how/if to handle multiple state changes in a frame?
            }
        }
        else
        {
            // As long as the key is down
            if (m_currentInputState.keyCodes[uiKeycode].isDown)
            {
                input_callback_func* cbFunc = m_callbacks_keyDownRepeat[uiKeycode];
                if (cbFunc)
                {
                    cbFunc(1);
                }
            }
        }
        // TODO: key upstroke callbacks
    }

    for (uint32 uiMousecode = 0; uiMousecode < Platform::Mousecode::eMax; ++uiMousecode)
    {
        bool mouseMove = uiMousecode == Platform::Mousecode::eMouseMoveVertical || uiMousecode == Platform::Mousecode::eMouseMoveHorizontal;
        int32 disp = m_currentInputState.mouseCodes[uiMousecode].displacement;
        if (mouseMove && disp != 0)
        {
            input_callback_func* cbFunc = m_callbacks_mouse[uiMousecode];
            if (cbFunc)
            {
                cbFunc((uint32)disp);
            }
        }
    }
}

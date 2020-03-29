#pragma once

#include "CommonTypes.h"

// forwards
struct ImGuiIO;

// debug gui window base class
class DebugUiWindow: public cxx::noncopyable
{
    friend class DebugUiManager;

public:
    // @param windowName: Window name, must be unique, cannot change once specified
    DebugUiWindow();
    virtual ~DebugUiWindow();

    // process window logic
    // @param imguiContext: Internal imgui context
    virtual void DoUI(ImGuiIO& imguiContext) = 0;
    
    // show or hide window
    // @param isShown: Flag
    void SetWindowShown(bool isShown);
    void ToggleWindowShown();

    // test whether window is shown
    inline bool IsWindowShown() const { return mWindowShown; }

protected:
    bool mWindowShown = false;
};

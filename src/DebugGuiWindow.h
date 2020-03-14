#pragma once

#include "CommonTypes.h"

// forwards
struct ImGuiIO;

// debug gui window base class
class DebugGuiWindow: public cxx::noncopyable
{
    friend class DebugGuiManager;

public:
    const std::string mWindowName;

public:
    // @param windowName: Window name, must be unique, cannot change once specified
    DebugGuiWindow(const std::string& windowName);
    virtual ~DebugGuiWindow();

    // process window logic
    // @param imguiContext: Internal imgui context
    virtual void DoUI(ImGuiIO& imguiContext) = 0;
    
    // one time window init
    virtual void DoInit(ImGuiIO& imguiContext);

    // show or hide window
    // @param isShown: Flag
    inline void SetWindowShown(bool isShown)
    {
        mWindowShown = isShown;
    }
    
    inline void ToggleWindowShown()
    {
        SetWindowShown(!mWindowShown);
    }

    inline bool IsWindowShown() const { return mWindowShown; }

protected:

    // window params

    float mBackgroundAlpha      = 1.0f;

    // style flags

    bool mWindowShown           = false;
    bool mWindowNoTitleBar      = false;
    bool mWindowNoResize        = false;
    bool mWindowNoMove          = false;
    bool mWindowNoCloseButton   = false;
    bool mWindowNoCollapse      = false;
    bool mAlwaysAutoResize      = false;
    bool mWindowNoBackground    = false;
    bool mWindowNoMouseInput    = false;
    bool mNoFocusOnAppearing    = false;
    bool mNoNavigation          = false;
};

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
    DebugGuiWindow(const std::string& windowName, const Point2D& initialPosition, const Size2D& initialSize);
    virtual ~DebugGuiWindow();

    // process window logic
    // @param imguiContext: Internal imgui context
    virtual void DoUI(ImGuiIO& imguiContext) = 0;

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
    // initial show params
    Point2D mInitialPosition;
    Size2D mInitialSize;

    float mBackgroundAlpha      = 1.0f;

    // debug window flags
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

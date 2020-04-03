#pragma once

// forwards
struct ImGuiIO;

// debug gui window base class
class DebugUiWindow: public cxx::noncopyable
{
public:
    // readonly
    bool mWindowShown = false;

public:
    virtual ~DebugUiWindow();

    // process window logic
    // @param imguiContext: Internal imgui context
    virtual void DoUI(ImGuiIO& imguiContext) = 0;
    
    // show or hide window
    // @param isShown: Flag
    void SetWindowShown(bool isShown);
    void ToggleWindowShown();
};

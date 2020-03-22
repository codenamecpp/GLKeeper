#pragma once

#include "DebugGuiWindow.h"

// debug statistics window
class FpsWindow: public DebugGuiWindow
{
public:
    FpsWindow();

private:
    // overrides DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;
    void DoInit(ImGuiIO& imguiContext) override;
};

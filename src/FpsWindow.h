#pragma once

#include "DebugUiWindow.h"

// debug statistics window
class FpsWindow: public DebugUiWindow
{
public:
    FpsWindow();

private:
    // overrides DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;
};

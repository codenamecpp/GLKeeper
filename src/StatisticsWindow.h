#pragma once

#include "DebugUIWindow.h"

// debug statistics window
class StatisticsWindow: public DebugUIWindow
{
public:
    StatisticsWindow();

private:
    // overrides DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;
};

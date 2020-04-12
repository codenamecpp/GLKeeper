#pragma once

#include "ToolsUIWindow.h"

// scene statistics window
class ToolsUISceneStatisticsWindow: public ToolsUIWindow
{
public:
    ToolsUISceneStatisticsWindow();

private:
    // overrides ToolsUIWindow
    void DoUI(ImGuiIO& imguiContext) override;
};

#pragma once

#include "ToolsUIWindow.h"

// layouts edit window
class ToolsUILayoutsEditWindow: public ToolsUIWindow
{
public:
    ToolsUILayoutsEditWindow() = default;

protected:
    // override ToolsUIWindow
    void DoUI(ImGuiIO& imguiContext) override;
};
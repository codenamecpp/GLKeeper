#pragma once

#include "ToolsUIWindow.h"

// forwards
struct ImGuiInputTextCallbackData;

// console window
class ToolsUIConsoleWindow: public ToolsUIWindow
{
public:
    bool mAutoScroll = true;
    bool mScrollToBottom = false;

public:
    ToolsUIConsoleWindow();

private:
    // override ToolsUIWindow
    void DoUI(ImGuiIO& imguiContext) override;

    // internals
    int TextEditCallback(ImGuiInputTextCallbackData* data);
    void Exec();
    void MoveInputToHistory();

private:
    std::string mInputString;
    std::deque<std::string> mHistory;
    int mHistoryPos = -1; // -1: new line, 0..History.Size-1 browsing history.
};

extern ToolsUIConsoleWindow gConsoleWindow;
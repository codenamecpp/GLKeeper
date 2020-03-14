#pragma once

#include "DebugGuiWindow.h"

// forwards
struct ImGuiInputTextCallbackData;

// debug console window
class DebugConsoleWindow: public DebugGuiWindow
{
public:
    bool mAutoScroll = true;
    bool mScrollToBottom = false;

public:
    DebugConsoleWindow();

private:
    // override DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;

    // internals
    int TextEditCallback(ImGuiInputTextCallbackData* data);

private:
    std::vector<char> mInputBuffer;
    std::vector<std::string> mCommands;
    std::vector<std::string> mHistory;
    int mHistoryPos = -1; // -1: new line, 0..History.Size-1 browsing history.
};

extern DebugConsoleWindow gConsoleWindow;
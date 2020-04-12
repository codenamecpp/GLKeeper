#include "pch.h"
#include "ToolsUIManager.h"
#include "ToolsUIWindow.h"

ToolsUIWindow::~ToolsUIWindow()
{
}

void ToolsUIWindow::SetWindowShown(bool isShown)
{
    mWindowShown = isShown;
}

void ToolsUIWindow::ToggleWindowShown()
{
    SetWindowShown(!mWindowShown);
}
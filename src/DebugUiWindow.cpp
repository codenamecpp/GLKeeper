#include "pch.h"
#include "DebugUIManager.h"
#include "DebugUIWindow.h"

DebugUIWindow::~DebugUIWindow()
{
}

void DebugUIWindow::SetWindowShown(bool isShown)
{
    mWindowShown = isShown;
}

void DebugUIWindow::ToggleWindowShown()
{
    SetWindowShown(!mWindowShown);
}
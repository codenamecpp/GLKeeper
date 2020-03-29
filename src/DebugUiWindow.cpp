#include "pch.h"
#include "DebugUiManager.h"
#include "DebugUiWindow.h"

DebugUiWindow::~DebugUiWindow()
{
}

void DebugUiWindow::SetWindowShown(bool isShown)
{
    mWindowShown = isShown;
}

void DebugUiWindow::ToggleWindowShown()
{
    SetWindowShown(!mWindowShown);
}
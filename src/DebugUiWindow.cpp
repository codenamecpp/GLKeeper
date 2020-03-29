#include "pch.h"
#include "DebugUiManager.h"
#include "DebugUiWindow.h"

cxx::intrusive_list<DebugUiWindow> gAllWindowsList;

DebugUiWindow::DebugUiWindow()
{
}

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
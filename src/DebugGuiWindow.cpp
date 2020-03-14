#include "pch.h"
#include "DebugGuiManager.h"
#include "DebugGuiWindow.h"

cxx::intrusive_list<DebugGuiWindow> gAllWindowsList;

DebugGuiWindow::DebugGuiWindow(const std::string& windowName)
    : mWindowName(windowName)
{
}

DebugGuiWindow::~DebugGuiWindow()
{
}

void DebugGuiWindow::DoInit(ImGuiIO& imguiContext)
{
}

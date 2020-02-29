#include "pch.h"
#include "DebugGuiManager.h"
#include "DebugGuiWindow.h"

cxx::intrusive_list<DebugGuiWindow> gAllWindowsList;

DebugGuiWindow::DebugGuiWindow(const std::string& windowName)
    : mWindowName(windowName)
    , mInitialPosition(0, 0)
    , mInitialSize(0, 0)
{
}

DebugGuiWindow::DebugGuiWindow(const std::string& windowName, const Point2D& initialPosition, const Size2D& initialSize)
    : mWindowName(windowName)
    , mInitialPosition(initialPosition)
    , mInitialSize(initialSize)
{
}

DebugGuiWindow::~DebugGuiWindow()
{
}
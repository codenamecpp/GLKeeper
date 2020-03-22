#include "pch.h"
#include "imgui.h"
#include "FpsWindow.h"

FpsWindow::FpsWindow(): DebugGuiWindow("Fps Stats")
{
    mBackgroundAlpha = 0.35f;

    mWindowNoResize = true;
    mWindowNoTitleBar = true;
    mAlwaysAutoResize = true;
    mNoFocusOnAppearing = true;
    mNoNavigation = true;
}

void FpsWindow::DoUI(ImGuiIO& imguiContext)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frame Time: %.3f ms (%.1f FPS)", 1000.0f / imguiContext.Framerate, imguiContext.Framerate);


}

void FpsWindow::DoInit(ImGuiIO& imguiContext)
{
}
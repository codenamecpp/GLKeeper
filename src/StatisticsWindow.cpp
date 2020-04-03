#include "pch.h"
#include "imgui.h"
#include "StatisticsWindow.h"

StatisticsWindow::StatisticsWindow()
{
}

void StatisticsWindow::DoUI(ImGuiIO& imguiContext)
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (!ImGui::Begin("Fps Stats", nullptr, windowFlags))
    {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frame Time: %.3f ms (%.1f FPS)", 1000.0f / imguiContext.Framerate, imguiContext.Framerate);
    ImGui::End();
}

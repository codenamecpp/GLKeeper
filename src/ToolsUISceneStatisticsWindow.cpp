#include "pch.h"
#include "3rd_party/imgui.h"
#include "ToolsUISceneStatisticsWindow.h"
#include "GameMain.h"
#include "TerrainTile.h"
#include "TerrainManager.h"

ToolsUISceneStatisticsWindow::ToolsUISceneStatisticsWindow()
{
}

void ToolsUISceneStatisticsWindow::DoUI(ImGuiIO& imguiContext)
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

    // show hovered tile info
    if (gGameMain.IsGameplayGamestate())
    {
        if (TerrainTile* currentTile = gGameMain.mGameplayGamestate.mMapInteractionControl.mHoveredTile)
        {
            TerrainDefinition* terrainDefinition = currentTile->GetTerrain();
            const char* owner = cxx::enum_to_string(currentTile->mOwnerID);
            ImGui::Text("Hovered tile: (%d, %d) (%s) (%s)", 
                currentTile->mTileLocation.x, 
                currentTile->mTileLocation.y, terrainDefinition->mName.c_str(), owner);
        }
    }

    ImGui::End();
}

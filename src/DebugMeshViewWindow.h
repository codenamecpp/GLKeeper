#pragma once

#include "DebugGuiWindow.h"
#include "SceneDefs.h"

class MeshViewGamestate;

// imgui internals
struct ImGuiTextFilter;

// mesh view ui
class DebugMeshViewWindow: public DebugGuiWindow
{
public:
    DebugMeshViewWindow();
    ~DebugMeshViewWindow();

    void Setup(MeshViewGamestate* meshviewGamestate);

private:
    // override DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;

    void UpdateFilteredElementsList();
    void LoadModelsList();
    void ChangeModelAsset(const char* assetName);

private:
    int mListSelection = 0; // current selection item index in filtered list
    std::vector<const char*> mAllModelsList;
    std::vector<const char*> mFilteredList;
    ImGuiTextFilter* mModelsListFilter = nullptr;
    MeshViewGamestate* mMeshViewGamestate = nullptr;
};


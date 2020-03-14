#pragma once

#include "DebugGuiWindow.h"
#include "SceneDefs.h"

// imgui internals
struct ImGuiTextFilter;

// mesh view ui
class DebugMeshViewWindow: public DebugGuiWindow
{
public:
    DebugMeshViewWindow();
    ~DebugMeshViewWindow();

    void LoadModelsList();
    void SetAnimatingObject(AnimatingModel* animatingModel);

private:
    // override DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;

    void UpdateFilteredElementsList();
    void ChangeModelAsset(const char* assetName);

private:
    int mListSelection = 0; // current selection item index in filtered list
    std::vector<const char*> mAllModelsList;
    std::vector<const char*> mFilteredList;
    ImGuiTextFilter* mModelsListFilter = nullptr;
    AnimatingModel* mAnimatingModel = nullptr;
};


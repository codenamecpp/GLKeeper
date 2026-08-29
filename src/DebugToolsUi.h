#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ToolsUi.h"
#include "RenderView.h"
#include "AnimatingMeshObject.h"

//////////////////////////////////////////////////////////////////////////

class DebugToolsUi final: public ToolsUi
{
private:
    // override ToolsUi
    void DoUI(ImGuiIO& imguiContext, float deltaTime) override;
    void OnActivateUi() override;
    void OnDeactivateUi() override;

private:
    void DoCheatsTab(ImGuiIO& imguiContext);
    
    void EnableMeshPreview();
    void DisableMeshPreview(bool forceUnloadResources = false);
    enum MeshAnimType { MeshAnimType_Default, MeshAnimType_Start, MeshAnimType_End };
    void SetMeshPreview(const ArtResourceDefinition& def, MeshAnimType animType = MeshAnimType_Default);
    void UpdateMeshPreview(float deltaTime);
    void SetMeshRotation(cxx::angle_t meshRotation);

    void DoObjectDefsTab(ImGuiIO& imguiContext);
    void DoCreatueDefsTab(ImGuiIO& imguiContext);
    void DoMeshPreviewTab(ImGuiIO& imguiContext);
    void DoArtResourceDef(ImGuiIO& imguiContext, const ArtResourceDefinition& def);

    void SetupScenarioDefs();
    void ClearScenarioDefs();

    void ConfigurePreviewCameraProjection(Camera& camera);

    void ChangeGoldAmount(long amount);

private:
    cxx::uniqueptr<AnimatingMeshObject> mPreviewMeshObject;
    cxx::uniqueptr<RenderView> mPreviewMeshRenderView;

    bool mIsMeshPreviewActive = false;
    bool mRotatePreviewMesh = true;
    cxx::angle_t mPreviewMeshRotation;

    // scenario objects
    std::vector<GameObjectDefinition> mObjectDefsList;
    std::vector<CreatureDefinition> mCreatureDefsList;
};
#pragma once

#include "DebugRenderer.h"
#include "AnimModelsRenderer.h"
#include "SceneRenderList.h"
#include "TerrainMeshRenderer.h"
#include "WaterLavaMeshRenderer.h"
#include "ProcMeshRenderer.h"
#include "GuiRenderer.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderManager: public cxx::noncopyable
{
    friend class RenderProgram;
    friend class MeshMaterial;

public:
    // readonly
    AnimModelsRenderer mAnimatingModelsRenderer;
    TerrainMeshRenderer mTerrainMeshRenderer;
    WaterLavaMeshRenderer mWaterLavaMeshRenderer;
    ProcMeshRenderer mProcMeshRenderer;

public:
    // setup rendering system internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();

    // register object for rendering on current render frame
    void RegisterObjectForRendering(SceneObject* object);

    // render game frame routine
    void RenderFrame();

    // process single frame logic
    void UpdateFrame();

    // force reload all render programs
    void ReloadRenderPrograms();

    void HandleScreenResolutionChanged();

private:
    void DrawScene();

    void HandleRenderProgramLoad(RenderProgram* renderProgram);
    void HandleRenderProgramFree(RenderProgram* renderProgram);

    void HandleMaterialActivate(MeshMaterial* material);

private:
    DebugRenderer mDebugRenderer;
    GuiRenderer mGuiRenderer;
    SceneRenderList mSceneRenderList;
    RenderProgram* mActiveRenderProgram = nullptr;
    std::vector<RenderProgram*> mLoadedRenderProgramsList;
};

extern RenderManager gRenderManager;
#pragma once

#include "DebugRenderer.h"
#include "AnimatingModelsRenderer.h"
#include "SceneRenderList.h"
#include "TerrainMeshRenderer.h"
#include "WaterLavaMeshRenderer.h"
#include "GuiRenderer.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderManager: public cxx::noncopyable
{
    friend RenderProgram;

public:
    // readonly
    AnimatingModelsRenderer mAnimatingModelsRenderer;
    TerrainMeshRenderer mTerrainMeshRenderer;
    WaterLavaMeshRenderer mWaterLavaMeshRenderer;

public:
    // setup rendering system internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();

    // register game object for rendering on current render frame
    // @param gameObject: Object
    void RegisterSceneObjectForRendering(GameObject* gameObject);

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

private:
    DebugRenderer mDebugRenderer;
    GuiRenderer mGuiRenderer;
    SceneRenderList mSceneRenderList;
    RenderProgram* mActiveRenderProgram = nullptr;
    std::vector<RenderProgram*> mLoadedRenderProgramsList;
};

extern RenderManager gRenderManager;
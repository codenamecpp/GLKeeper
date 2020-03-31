#pragma once

#include "GraphicsDefs.h"
#include "GuiRenderProgram.h"
#include "DebugRenderer.h"
#include "AnimatingModelsRenderer.h"
#include "SceneRenderList.h"
#include "TerrainMeshRenderer.h"
#include "WaterLavaMeshRenderer.h"

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

    // register scene object for rendering on current render frame
    // @param sceneObject: Object
    void RegisterSceneObjectForRendering(SceneObject* sceneObject);

    // render game frame routine
    void RenderFrame();

    // process single frame logic
    void UpdateFrame();

    // force reload all render programs
    void ReloadRenderPrograms();

private:
    // begin/end render gui geometry
    void Enter2D();
    void Leave2D();

    void DrawScene();

    void HandleRenderProgramLoad(RenderProgram* renderProgram);
    void HandleRenderProgramFree(RenderProgram* renderProgram);

private:
    GuiRenderProgram mGuiRenderProgram;
    DebugRenderer mDebugRenderer;

    glm::mat4 mTransformationMatrix2D;

    SceneRenderList mSceneRenderList;

    RenderProgram* mActiveRenderProgram = nullptr;
    std::vector<RenderProgram*> mLoadedRenderProgramsList;
};

extern RenderManager gRenderManager;
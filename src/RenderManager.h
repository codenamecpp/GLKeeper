#pragma once

#include "GraphicsDefs.h"
#include "GuiRenderProgram.h"
#include "DebugRenderer.h"
#include "AnimatingModelsRenderer.h"
#include "SceneRenderList.h"
#include "TerrainMeshRenderer.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderManager: public cxx::noncopyable
{
public:
    // readonly
    AnimatingModelsRenderer mAnimatingModelsRenderer;
    TerrainMeshRenderer mTerrainMeshRenderer;

    RenderProgram* mActiveRenderProgram = nullptr;

public:
    // setup rendering system internal resources
    // @returns false on error
    bool Initialize();
    void Deinit();

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

private:
    GuiRenderProgram mGuiRenderProgram;
    DebugRenderer mDebugRenderer;

    glm::mat4 mTransformationMatrix2D;

    SceneRenderList mSceneRenderList;
};

extern RenderManager gRenderManager;
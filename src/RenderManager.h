#pragma once

#include "GraphicsDefs.h"
#include "GuiRenderProgram.h"
#include "DebugRenderer.h"
#include "ModelsRenderer.h"
#include "SceneRenderList.h"

// master render system, it is intended to manage rendering pipeline of the game
class RenderManager: public cxx::noncopyable
{
public:
    // readonly
    ModelsRenderer mModelsRenderer;

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
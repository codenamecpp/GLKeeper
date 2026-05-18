#pragma once

#include "RenderDevice.h"
#include "IDebugVisualizer.h"
#include "TerrainRenderer.h"
#include "EnvironmentMeshRenderer.h"
#include "UiRenderContext.h"
#include "TileSelectionOutline.h"
#include "AnimatingMeshRenderer.h"
#include "ProceduralMeshRenderer.h"
#include "ShaderProgram.h"
#include "SceneDefs.h"
#include "RenderView.h"

//////////////////////////////////////////////////////////////////////////
// RenderEngine class
// It is intended to manage rendering pipeline of the game
//////////////////////////////////////////////////////////////////////////

class GameRenderManager
{
public:
    TerrainRenderer mTerrainRenderer;

public:
    GameRenderManager();

    // First time render engine initialization / finalization
    // All shaders, buffers and other graphics resources might be loaded here
    // Return false on error
    bool Initialize();
    void Shutdown();

    // Render game frame routine
    void RenderFrame();

    // Register/Unregister debug visualizer for processing
    // Do not perform these operations while rendering in progress
    // @param theVisualizer: Entity
    void RegisterDebugVisualizer(IDebugVisualizer* theVisualizer);
    void UnregisterDebugVisualizer(IDebugVisualizer* theVisualizer);

    // create new render view of game world
    RenderView* CreateRenderView();
    void DestroyRenderView(RenderView* renderView);

private:
    void RenderWorld(Camera& camera, Scene& scene);
    void RenderScene(Camera& camera, SceneRenderLists& renderLists);
    void RenderCustomViews(Scene& scene);

private:
    DebugRenderer mDebugRenderer;
    UiRenderContext mUiRenderContext;
    SceneRenderLists mRenderLists;
    std::vector<IDebugVisualizer*> mDebugVisializers;
    std::vector<std::unique_ptr<RenderView>> mRenderViews;

    AnimatingMeshRenderer mAnimatingMeshlRenderer;
    EnvironmentMeshRenderer mEnvironmentMeshRenderer;
    ProceduralMeshRenderer mProceduralMeshRenderer;
};

extern GameRenderManager gGameRenderer;
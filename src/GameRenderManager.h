#pragma once

#include "RenderDevice.h"
#include "IDebugVisualizer.h"
#include "TerrainRenderer.h"
#include "EnvironmentMeshRenderer.h"
#include "UiRenderContext.h"
#include "MapSelectionCursor.h"
#include "AnimatingMeshRenderer.h"
#include "ProceduralMeshRenderer.h"
#include "ShaderProgram.h"
#include "SceneDefs.h"

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
    void UnRegisterDebugVisualizer(IDebugVisualizer* theVisualizer);

    // create new render view of game world
    cxx::uniqueptr<RenderView> CreateRenderView();

private:
    void RenderWorld(Scene& scene, eRenderLayer renderLayer);
    void RenderWorld(Camera& camera, Scene& scene, eRenderLayer renderLayer);
    void RenderScene(Camera& camera, SceneRenderLists& renderLists);

    void UnRegisterRenderView(RenderView* renderView);

private:
    DebugRenderer mDebugRenderer;
    UiRenderContext mUiRenderContext;
    SceneRenderLists mRenderLists;
    std::vector<IDebugVisualizer*> mDebugVisializers;
    std::vector<RenderView*> mRenderViews;

    AnimatingMeshRenderer mAnimatingMeshRenderer;
    EnvironmentMeshRenderer mEnvironmentMeshRenderer;
    ProceduralMeshRenderer mProceduralMeshRenderer;
    ISceneObjectRenderer* mSceneObjectRenderers[eSceneObjectType_COUNT];
};

//////////////////////////////////////////////////////////////////////////

extern GameRenderManager gGameRenderer;

//////////////////////////////////////////////////////////////////////////
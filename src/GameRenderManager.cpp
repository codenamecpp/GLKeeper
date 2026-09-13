#include "stdafx.h"
#include "GameRenderManager.h"
#include "GameMain.h"
#include "ShadersManager.h"
#include "GameWorld.h"
#include "Scene.h"
#include "SimplePool.h"
#include "RenderView.h"
#include "UiManager.h"
#include "ToolsUiManager.h"

GameRenderManager gGameRenderer;

GameRenderManager::GameRenderManager() 
    : mDebugRenderer()
    , mTerrainRenderer()
    , mEnvironmentMeshRenderer()
    , mUiRenderContext()
{}

bool GameRenderManager::Initialize()
{
    // load graphics resources
    if (!mUiRenderContext.Initialize() || 
        !mDebugRenderer.Initialize() || 
        !mTerrainRenderer.Initialize() || 
        !mEnvironmentMeshRenderer.Initialize() || 
        !mAnimatingMeshlRenderer.Initialize() ||
        !mProceduralMeshRenderer.Initialize())
    {
        Shutdown();
        return false;
    }
    return true;
}

void GameRenderManager::Shutdown()
{
    mProceduralMeshRenderer.Shutdown();
    mAnimatingMeshlRenderer.Shutdown();
    mUiRenderContext.Deinit();
    mEnvironmentMeshRenderer.Shutdown();
    mTerrainRenderer.Shutdown();
    mDebugRenderer.Shutdown();
    mRenderLists.Clear();
    mRenderViews.clear();
}

void GameRenderManager::RenderFrame()
{
    gRenderDevice.BeginFrame();

    gRenderDevice.SetClearColor(COLOR_BLACK);
    gRenderDevice.ClearScreen(eDeviceClear_ColorBuffer);

    // world / world overlay
    RenderWorld(gScene, eSceneRenderLayer_World);
    RenderWorld(gScene, eSceneRenderLayer_WorldOverlay);

    // Debug information
    if (!mDebugVisializers.empty())
    {
        mDebugRenderer.BeginFrame(gScene.GetCamera());
        for (IDebugVisualizer* visualizer : mDebugVisializers) 
        {
            visualizer->OnDebugDraw(mDebugRenderer);
        }
        mDebugRenderer.EndFrame();
    }

    // render gui
    mUiRenderContext.BeginFrame();
    gUiManager.RenderFrame(mUiRenderContext);
    mUiRenderContext.EndFrame();

    // overlays
    RenderWorld(gScene, eSceneRenderLayer_UiOverlay);
    RenderWorld(gScene, eSceneRenderLayer_DebugOverlay);

    // ui overlay
    mUiRenderContext.BeginFrame();
    gUiManager.RenderFrameOverlay(mUiRenderContext);
    mUiRenderContext.EndFrame();

    // tools
    if (gToolsUiManager.IsInitialized())
    {
        mUiRenderContext.BeginFrame();
        gToolsUiManager.RenderFrame();
        mUiRenderContext.EndFrame();
    }

    gRenderDevice.EndFrame();
}

void GameRenderManager::RenderWorld(Camera& camera, Scene& scene, eSceneRenderLayer renderLayer)
{
    camera.ComputeMatricesAndFrustum(gRenderDevice.GetViewport());

    mAnimatingMeshlRenderer.BeginFrame();
    mEnvironmentMeshRenderer.BeginFrame();
    mProceduralMeshRenderer.BeginFrame();

    if (renderLayer == eSceneRenderLayer_World)
    {
        mTerrainRenderer.Render(camera);
    }

    mRenderLists.Clear();
    scene.CollectObjectsForRender(camera, mRenderLists);
    RenderScene(camera, mRenderLists);
    mRenderLists.Clear();

    mProceduralMeshRenderer.EndFrame();
    mEnvironmentMeshRenderer.EndFrame();
    mAnimatingMeshlRenderer.EndFrame();
}

void GameRenderManager::RenderWorld(Scene& scene, eSceneRenderLayer renderLayer)
{
    // main
    {
        Camera& camera = gScene.GetCamera();
        if (camera.mRenderLayers.Contains(renderLayer))
        {
            gRenderDevice.ClearScreen(eDeviceClear_DepthBuffer);
            RenderWorld(camera, scene, renderLayer);
        }
    }

    // custom views
    for (RenderView* roller: mRenderViews)
    {
        if (!roller->IsActive())
            continue;

        Camera& camera = roller->GetCamera();
        if (!camera.mRenderLayers.Contains(renderLayer))
        {
            continue;
        }
        gRenderDevice.ClearScreen(eDeviceClear_DepthBuffer);
        RenderWorld(camera, scene, renderLayer);
    }
}

void GameRenderManager::RenderScene(Camera& camera, SceneRenderLists& renderLists)
{
    // opaque pass
    // sort by object type

    auto& opaqueList = renderLists.mListsPerPass[eRenderPass_Opaque];
    if (!opaqueList.empty())
    {
        std::sort(opaqueList.begin(), opaqueList.end(),
            [](const SceneRenderLists::Entry& lhs, const SceneRenderLists::Entry& rhs)
            {
                if (lhs.mWaterLavaMesh)
                    return (rhs.mWaterLavaMesh == nullptr) || (lhs.mWaterLavaMesh < rhs.mWaterLavaMesh);

                if (lhs.mAnimatingMesh)
                    return (rhs.mAnimatingMesh == nullptr) || (lhs.mAnimatingMesh < rhs.mAnimatingMesh);

                if (lhs.mProceduralMesh)
                    return (rhs.mProceduralMesh == nullptr) || (lhs.mProceduralMesh < rhs.mProceduralMesh);

                // add more here
                cxx_assert(false);

                return lhs.mDistanceToCamera2 > rhs.mDistanceToCamera2;
            });
    }

    // translucent pass
    // sort by camera distance, from far to near

    auto& translucentList = renderLists.mListsPerPass[eRenderPass_Translucent];
    if (!translucentList.empty())
    {
        std::sort(translucentList.begin(), translucentList.end(), 
                [](const SceneRenderLists::Entry& lhs, const SceneRenderLists::Entry& rhs)
                {
                    return lhs.mDistanceToCamera2 < rhs.mDistanceToCamera2;
                });
    }

    // process render lists in batches

    for (eRenderPass currentPass: {eRenderPass_Opaque, eRenderPass_Translucent})
    {
        const auto& currentPassList = renderLists.mListsPerPass[currentPass];
        for (auto roller_it = currentPassList.begin(), end_it = currentPassList.end(); roller_it != end_it ;)
        {
            // process water lava meshes
            if (roller_it->mWaterLavaMesh)
            {
                mEnvironmentMeshRenderer.BeginBatch(camera);
                do
                {
                    if (!gDebug.mNoDrawWaterLava)
                    {
                        mEnvironmentMeshRenderer.RenderInstance(currentPass, *roller_it->mWaterLavaMesh);
                    }
                    ++roller_it; // advance
                } 
                while ((roller_it != end_it) && roller_it->mWaterLavaMesh);
                mEnvironmentMeshRenderer.EndBatch();
                continue;
            }

            // process animated models
            if (roller_it->mAnimatingMesh)
            {
                mAnimatingMeshlRenderer.BeginBatch(camera);
                do
                {
                    mAnimatingMeshlRenderer.RenderInstance(currentPass, *roller_it->mAnimatingMesh);
                    ++roller_it; // advance
                }
                while ((roller_it != end_it) && roller_it->mAnimatingMesh);
                mAnimatingMeshlRenderer.EndBatch();
                continue;
            }

            // process procedural mesh objects
            if (roller_it->mProceduralMesh)
            {
                mProceduralMeshRenderer.BeginBatch(camera);
                do
                {
                    mProceduralMeshRenderer.RenderInstance(currentPass, *roller_it->mProceduralMesh);
                    ++roller_it; // advance
                }
                while ((roller_it != end_it) && roller_it->mProceduralMesh);
                mProceduralMeshRenderer.EndBatch();
                continue;
            }

            // add more here
            cxx_assert(false);

            ++roller_it;
        }
    }

    // done
}

void GameRenderManager::UnRegisterRenderView(RenderView* renderView)
{
    cxx_assert(renderView);
    cxx::erase_elements(mRenderViews, renderView);
}

void GameRenderManager::RegisterDebugVisualizer(IDebugVisualizer* theVisualizer)
{
    cxx_assert(theVisualizer);
    if (theVisualizer && !cxx::contains(mDebugVisializers, theVisualizer))
    {
        mDebugVisializers.push_back(theVisualizer);
    }
}

void GameRenderManager::UnRegisterDebugVisualizer(IDebugVisualizer* theVisualizer)
{
    cxx::erase(mDebugVisializers, theVisualizer);
}

cxx::uniqueptr<RenderView> GameRenderManager::CreateRenderView()
{
    static SimplePool<RenderView> viewsPool = (
        [](RenderView* renderView)
        {
            renderView->OnRecycle();
        });

    RenderView* renderViewPtr = viewsPool.Acquire();
    // sanity check
    cxx_assert(!cxx::contains(mRenderViews, renderViewPtr));
    // register
    mRenderViews.push_back(renderViewPtr);
    return std::move(cxx::uniqueptr<RenderView> (renderViewPtr, [](RenderView* renderView)
        {
            if (renderView)
            {
                gGameRenderer.UnRegisterRenderView(renderView);
                viewsPool.Return(renderView);
            }
        }));
}
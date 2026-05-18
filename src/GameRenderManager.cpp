#include "stdafx.h"
#include "GameRenderManager.h"
#include "GameMain.h"
#include "UiWidgetManager.h"
#include "ShadersManager.h"
#include "GameWorld.h"
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
    gRenderDevice.ClearScreen();

    Scene& scene = gGame.GetGameSession().GetGameWorld().GetScene();
    Camera& camera = scene.GetCamera();
    RenderWorld(camera, scene);

    // Debug information
    if (!mDebugVisializers.empty())
    {
        mDebugRenderer.BeginFrame(camera);
        for (IDebugVisualizer* visualizer : mDebugVisializers) 
        {
            visualizer->OnDebugDraw(mDebugRenderer);
        }
        mDebugRenderer.EndFrame();
    }

    RenderCustomViews(scene);

    // render gui
    mUiRenderContext.BeginFrame();
    gWidgetManager.RenderFrame(mUiRenderContext);
    mUiRenderContext.EndFrame();

    // render tools ui
    mUiRenderContext.BeginFrame();
    gToolsUiManager.RenderFrame();
    mUiRenderContext.EndFrame();

    gRenderDevice.EndFrame();
}

void GameRenderManager::RenderWorld(Camera& camera, Scene& scene)
{
    camera.ComputeMatricesAndFrustum(gRenderDevice.GetViewport());

    mAnimatingMeshlRenderer.BeginFrame(camera);
    mEnvironmentMeshRenderer.BeginFrame(camera);
    mProceduralMeshRenderer.BeginFrame(camera);

    if ((camera.mRenderLayersMask & RenderLayer_WorldTerrain) != 0)
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
                mEnvironmentMeshRenderer.BeginBatch();
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
                mAnimatingMeshlRenderer.BeginBatch();
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
                mProceduralMeshRenderer.BeginBatch();
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

void GameRenderManager::RenderCustomViews(Scene& scene)
{
    if (mRenderViews.empty()) return;

    for (const auto& roller: mRenderViews)
    {
        if (!roller->IsActive()) continue;

        gRenderDevice.ClearScreen(eDeviceClear_DepthBuffer);
        RenderWorld(roller->GetCamera(), scene);
    }
}

void GameRenderManager::RegisterDebugVisualizer(IDebugVisualizer* theVisualizer)
{
    cxx_assert(theVisualizer);
    if (theVisualizer && !cxx::contains(mDebugVisializers, theVisualizer))
    {
        mDebugVisializers.push_back(theVisualizer);
    }
}

void GameRenderManager::UnregisterDebugVisualizer(IDebugVisualizer* theVisualizer)
{
    cxx::erase(mDebugVisializers, theVisualizer);
}

RenderView* GameRenderManager::CreateRenderView()
{
    auto& element = mRenderViews.emplace_back(std::make_unique<RenderView>());
    return element.get();
}

void GameRenderManager::DestroyRenderView(RenderView* renderView)
{
    int itemIndex = cxx::get_first_index_if(mRenderViews, [renderView](const std::unique_ptr<RenderView>& element)
        {
            return renderView == element.get();
        });
    cxx_assert(itemIndex != -1);
    if (itemIndex != -1)
    {
        mRenderViews.erase(mRenderViews.begin() + itemIndex);
    }
}

#include "pch.h"
#include "RenderManager.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "DebugGuiManager.h"
#include "GameMain.h"
#include "ConsoleVariable.h"
#include "RenderScene.h"
#include "SceneObject.h"

//////////////////////////////////////////////////////////////////////////

// cvars
CvarBoolean gCvarRender_DebugDrawEnabled ("r_debugDraw", true, "Enable debug draw", ConsoleVar_Debug | ConsoleVar_Renderer);
CvarBoolean gCvarRender_EnableAnimBlendFrames("r_animBlendFrames", true, "Smooth animations", ConsoleVar_Renderer);

//////////////////////////////////////////////////////////////////////////

RenderManager gRenderManager;

//////////////////////////////////////////////////////////////////////////

bool RenderManager::Initialize()
{
    if (!mGuiRenderProgram.LoadProgram() || !mAnimatingModelsRenderer.Initialize() || 
        !mTerrainMeshRenderer.Initialize() || !mWaterLavaMeshRenderer.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render manager");

        Deinit();
        return false;
    }

    if (!mDebugRenderer.Initialize())
    {
        debug_assert(false);
    }

    gGraphicsDevice.SetClearColor(Color32_GrimGray);

    gConsole.RegisterVariable(&gCvarRender_DebugDrawEnabled);
    gConsole.RegisterVariable(&gCvarRender_EnableAnimBlendFrames);

    return true;
}

void RenderManager::Deinit()
{
    gConsole.UnregisterVariable(&gCvarRender_DebugDrawEnabled);
    gConsole.UnregisterVariable(&gCvarRender_EnableAnimBlendFrames);

    mWaterLavaMeshRenderer.Deinit();
    mAnimatingModelsRenderer.Deinit();
    mTerrainMeshRenderer.Deinit();
    mDebugRenderer.Deinit();
    mGuiRenderProgram.FreeProgram();
    mSceneRenderList.Clear();

    mLoadedRenderProgramsList.clear();
}

void RenderManager::RenderFrame()
{
    gGraphicsDevice.ClearScreen();
    
    DrawScene();

    // debug draw
    if (gCvarRender_DebugDrawEnabled.mValue)
    {
        mDebugRenderer.RenderFrameBegin();
        gGameMain.DebugRenderFrame(mDebugRenderer);
        mDebugRenderer.RenderFrameEnd();
    }

    RenderStates prevRenderStates = gGraphicsDevice.mCurrentStates;

    Enter2D();
    gDebugGuiManager.RenderFrame();
    Leave2D();

    gGraphicsDevice.SetRenderStates(prevRenderStates);

    gGraphicsDevice.Present();
}

void RenderManager::UpdateFrame()
{

}

void RenderManager::ReloadRenderPrograms()
{
    std::vector<RenderProgram*> programs = mLoadedRenderProgramsList;

    for (RenderProgram* currProgram: programs)
    {
        currProgram->ReloadProgram();    
    }
}

void RenderManager::Enter2D()
{
    RenderStates guiRenderStates = RenderStates::GetForUI();
    gGraphicsDevice.SetRenderStates(guiRenderStates);

    Rect2D screenRect;
    screenRect.mX = 0;
    screenRect.mY = 0;
    screenRect.mSizeX = gGraphicsDevice.mViewportRect.mSizeX;
    screenRect.mSizeY = gGraphicsDevice.mViewportRect.mSizeY;

    mTransformationMatrix2D = glm::ortho(screenRect.mX * 1.0f, 
        (screenRect.mX + screenRect.mSizeX) * 1.0f, 
        (screenRect.mY + screenRect.mSizeY) * 1.0f, screenRect.mY * 1.0f);

    mGuiRenderProgram.ActivateProgram();
    mGuiRenderProgram.SetViewProjectionMatrix(mTransformationMatrix2D);
    // set scissor box
    gGraphicsDevice.SetScissorRect(screenRect);
}

void RenderManager::Leave2D()
{
    mGuiRenderProgram.DeactivateProgram();

    // restore scissor box
    Rect2D screenRect;
    screenRect.mX = 0;
    screenRect.mY = 0;
    screenRect.mSizeX = gGraphicsDevice.mViewportRect.mSizeX;
    screenRect.mSizeY = gGraphicsDevice.mViewportRect.mSizeY;
    gGraphicsDevice.SetScissorRect(screenRect);
}

void RenderManager::DrawScene()
{
    SceneRenderContext renderContext;

    gRenderScene.CollectObjectsForRendering(mSceneRenderList);

    // sort by renderable type
    mSceneRenderList.SortOpaquesByObjectType();

    // opaque pass
    renderContext.mCurrentPass = eRenderPass_Opaque;
    
    for (int i = 0; i < mSceneRenderList.mOpaqueElementsCount; ++i)
    {
        const auto& element = mSceneRenderList.mOpaqueElements[i];
        if (element.mAnimatingModel)
        {
            mAnimatingModelsRenderer.RenderModel(renderContext, element.mAnimatingModel);
        }
        if (element.mTerrainMesh)
        {
            mTerrainMeshRenderer.RenderTerrainMesh(renderContext, element.mTerrainMesh);
        }
        if (element.mWaterLavaMesh)
        {
            mWaterLavaMeshRenderer.RenderWaterLavaMesh(renderContext, element.mWaterLavaMesh);
        }
    }

    // translucent pass
    renderContext.mCurrentPass = eRenderPass_Translucent;

    // sort by camera distance, from far to near
    mSceneRenderList.SortTranslucentsByDistanceToCamera();

    for (int i = 0; i < mSceneRenderList.mTranslucentElementsCount; ++i)
    {
        const auto& element = mSceneRenderList.mTranslucentElements[i];
        if (element.mAnimatingModel)
        {
            mAnimatingModelsRenderer.RenderModel(renderContext, element.mAnimatingModel);
        }
        if (element.mTerrainMesh)
        {
            mTerrainMeshRenderer.RenderTerrainMesh(renderContext, element.mTerrainMesh);
        }
        if (element.mWaterLavaMesh)
        {
            mWaterLavaMeshRenderer.RenderWaterLavaMesh(renderContext, element.mWaterLavaMesh);
        }
    }

    mSceneRenderList.Clear();
}

void RenderManager::HandleRenderProgramLoad(RenderProgram* renderProgram)
{
    debug_assert(renderProgram);

    auto program_iter = std::find(mLoadedRenderProgramsList.begin(), mLoadedRenderProgramsList.end(), renderProgram);
    if (program_iter == mLoadedRenderProgramsList.end())
    {
        mLoadedRenderProgramsList.push_back(renderProgram);
    }
    else
    {
        debug_assert(false);
    }
}

void RenderManager::HandleRenderProgramFree(RenderProgram* renderProgram)
{
    debug_assert(renderProgram);

    auto program_iter = std::find(mLoadedRenderProgramsList.begin(), mLoadedRenderProgramsList.end(), renderProgram);
    if (program_iter != mLoadedRenderProgramsList.end())
    {
        mLoadedRenderProgramsList.erase(program_iter);
    }
    else
    {
        debug_assert(false);
    }
}
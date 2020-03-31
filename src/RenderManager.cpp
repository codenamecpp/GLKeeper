#include "pch.h"
#include "RenderManager.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "DebugUiManager.h"
#include "GameMain.h"
#include "ConsoleVariable.h"
#include "RenderScene.h"
#include "SceneObject.h"

//////////////////////////////////////////////////////////////////////////

// cvars
CvarBoolean gCvarRender_DebugDrawEnabled ("r_debugDraw", true, "Enable debug draw", ConsoleVar_Debug | ConsoleVar_Renderer);
CvarBoolean gCvarRender_EnableAnimBlendFrames("r_animBlendFrames", true, "Smooth animations", ConsoleVar_Renderer);
CvarBoolean gCVarRender_DrawWaterAndLava("r_drawWaterLava", true, "Draw water and lava", ConsoleVar_Renderer);
CvarBoolean gCVarRender_DrawTerrain("r_drawTerrain", true, "Draw terrain", ConsoleVar_Renderer);
CvarBoolean gCVarRender_DrawModels("r_drawModels", true, "Draw models", ConsoleVar_Renderer);

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
    gConsole.RegisterVariable(&gCVarRender_DrawTerrain);
    gConsole.RegisterVariable(&gCVarRender_DrawWaterAndLava);
    gConsole.RegisterVariable(&gCVarRender_DrawModels);

    return true;
}

void RenderManager::Deinit()
{
    gConsole.UnregisterVariable(&gCvarRender_DebugDrawEnabled);
    gConsole.UnregisterVariable(&gCvarRender_EnableAnimBlendFrames);
    gConsole.UnregisterVariable(&gCVarRender_DrawTerrain);
    gConsole.UnregisterVariable(&gCVarRender_DrawWaterAndLava);
    gConsole.UnregisterVariable(&gCVarRender_DrawModels);

    mWaterLavaMeshRenderer.Deinit();
    mAnimatingModelsRenderer.Deinit();
    mTerrainMeshRenderer.Deinit();
    mDebugRenderer.Deinit();
    mGuiRenderProgram.FreeProgram();
    mSceneRenderList.Clear();

    mLoadedRenderProgramsList.clear();
}

void RenderManager::RegisterSceneObjectForRendering(SceneObject* sceneObject)
{
    debug_assert(sceneObject);

    if (AnimatingModelComponent* component = sceneObject->GetAnimatingModelComponent())
    {
        bool hasOpaqueParts = false;
        bool hasTranslucentParts = false;

        for (const RenderMaterial& currMaterial: component->mSubmeshMaterials)
        {
            if (currMaterial.IsTransparent())
            {
                hasTranslucentParts = true;
            }
            else
            {
                hasOpaqueParts = true;
            }
        }

        if (hasTranslucentParts)
        {
            mSceneRenderList.RegisterRenderableComponent(eRenderPass_Translucent, component);
        }

        if (hasOpaqueParts)
        {
            mSceneRenderList.RegisterRenderableComponent(eRenderPass_Opaque, component);
        }
    }

    if (TerrainMeshComponent* component = sceneObject->GetTerrainMeshComponent())
    {
        bool hasOpaqueParts = false;
        bool hasTranslucentParts = false;

        for (const auto& currBatch: component->mBatchArray)
        {
            if (currBatch.mMaterial.IsTransparent())
            {
                hasTranslucentParts = true;
            }
            else
            {
                hasOpaqueParts = true;
            }
        }

        if (hasTranslucentParts)
        {
            mSceneRenderList.RegisterRenderableComponent(eRenderPass_Translucent, component);
        }

        if (hasOpaqueParts)
        {
            mSceneRenderList.RegisterRenderableComponent(eRenderPass_Opaque, component);
        }
    }

    if (WaterLavaMeshComponent* component = sceneObject->GetWaterLavaMeshComponent())
    {
        bool hasOpaqueParts = false;
        bool hasTranslucentParts = false;

        if (component->mMaterial.IsTransparent())
        {
            hasTranslucentParts = true;
        }
        else
        {
            hasOpaqueParts = true;
        }

        if (hasTranslucentParts)
        {
            mSceneRenderList.RegisterRenderableComponent(eRenderPass_Translucent, component);
        }

        if (hasOpaqueParts)
        {
            mSceneRenderList.RegisterRenderableComponent(eRenderPass_Opaque, component);
        }
    }
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
    gDebugUiManager.RenderFrame();
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

    gRenderScene.CollectObjectsForRendering();

    mSceneRenderList.SortOpaqueComponents();
    mSceneRenderList.SortTranslucentComponents();

    for (eRenderPass currRenderPass : {eRenderPass_Opaque, eRenderPass_Translucent})
    {
        const auto& currentList = mSceneRenderList.mComponentsForRenderPass[currRenderPass];
        for (int icurrentComponent = 0; icurrentComponent < currentList.mElementsCount; ++icurrentComponent)
        {
            SceneObjectComponent* currentComponent = currentList.mElements[icurrentComponent];
            if (currentComponent->IsRenderableComponent())
            {
                currentComponent->RenderFrame(renderContext);
            }
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
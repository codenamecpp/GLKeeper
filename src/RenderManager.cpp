#include "pch.h"
#include "RenderManager.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "ToolsUIManager.h"
#include "GameMain.h"
#include "ConsoleVariable.h"
#include "RenderScene.h"
#include "SceneObject.h"
#include "GuiManager.h"
#include "RenderableModel.h"
#include "RenderableWaterLavaMesh.h"
#include "RenderableTerrainMesh.h"
#include "TerrainManager.h"

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
    if (!mAnimatingModelsRenderer.Initialize() ||  !mTerrainMeshRenderer.Initialize() || 
        !mWaterLavaMeshRenderer.Initialize() || !mGuiRenderer.Initialize() || !mProcMeshRenderer.Initialize())
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

    mGuiRenderer.Deinit();
    mWaterLavaMeshRenderer.Deinit();
    mAnimatingModelsRenderer.Deinit();
    mTerrainMeshRenderer.Deinit();
    mProcMeshRenderer.Deinit();
    mDebugRenderer.Deinit();
    mSceneRenderList.Clear();

    mLoadedRenderProgramsList.clear();
}

void RenderManager::RegisterObjectForRendering(SceneObject* object)
{
    debug_assert(object);
    object->RegisterForRendering(mSceneRenderList);
}

void RenderManager::RenderFrame()
{
    // setup viewport area
    Rectangle viewportRect(0, 0, gGraphicsDevice.mScreenResolution.x, gGraphicsDevice.mScreenResolution.y);
    gGraphicsDevice.SetViewportRect(viewportRect);
    gGraphicsDevice.SetScissorRect(viewportRect);

    gGraphicsDevice.ClearScreen();

    gTerrainManager.PreRenderScene();
    
    // draw objects
    DrawScene();

    // debug draw
    if (gCvarRender_DebugDrawEnabled.mValue)
    {
        mDebugRenderer.RenderFrameBegin();
        gGameMain.DebugRenderFrame(mDebugRenderer);
        mDebugRenderer.RenderFrameEnd();
    }

    RenderStates prevRenderStates = gGraphicsDevice.mCurrentStates;

    // draw game ui
    mGuiRenderer.RenderFrameBegin();
    gGuiManager.RenderFrame(mGuiRenderer);
    mGuiRenderer.RenderFrameEnd();

    // draw debug ui
    mGuiRenderer.RenderFrameBegin();
    gToolsUIManager.RenderFrame();
    mGuiRenderer.RenderFrameEnd();

    // finish draw
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

void RenderManager::HandleScreenResolutionChanged()
{
    for (RenderProgram* currRenderProgram: mLoadedRenderProgramsList)
    {
        currRenderProgram->HandleScreenResolutionChanged();
    }
}

void RenderManager::DrawScene()
{
    SceneRenderContext renderContext;

    gRenderScene.CollectObjectsForRendering();

    mSceneRenderList.SortOpaque();
    mSceneRenderList.SortTranslucent();

    for (eRenderPass currRenderPass : {eRenderPass_Opaque, eRenderPass_Translucent})
    {
        const auto& currentList = mSceneRenderList.mObjectsForRenderPass[currRenderPass];
        for (int icurrentComponent = 0; icurrentComponent < currentList.mElementsCount; ++icurrentComponent)
        {
            SceneObject* currentObject = currentList.mElements[icurrentComponent];
            renderContext.mCurrentPass = currRenderPass;
            currentObject->RenderFrame(renderContext);
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

void RenderManager::HandleMaterialActivate(MeshMaterial* material)
{
    debug_assert(material);
    if (mActiveRenderProgram)
    {
        mActiveRenderProgram->SetMaterialColor(material->mMaterialColor);
    }
}

#include "pch.h"
#include "RenderManager.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "DebugGuiManager.h"
#include "GameMain.h"
#include "ConsoleVariable.h"
#include "RenderScene.h"

//////////////////////////////////////////////////////////////////////////

// cvars
CvarBoolean gCvarRender_DebugDrawEnabled ("r_debugDraw", true, "Enable debug draw", ConsoleVar_Debug | ConsoleVar_Renderer);

//////////////////////////////////////////////////////////////////////////

RenderManager gRenderManager;

//////////////////////////////////////////////////////////////////////////

bool RenderManager::Initialize()
{
    if (!mGuiRenderProgram.LoadProgram() || !mModelsRenderer.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render manager");

        Deinit();
        return false;
    }

    if (!mDebugRenderer.Initialize())
    {
        debug_assert(false);
    }

    gGraphicsDevice.SetClearColor(Color32_Black);

    return true;
}

void RenderManager::Deinit()
{
    mModelsRenderer.Deinit();
    mDebugRenderer.Deinit();
    mGuiRenderProgram.FreeProgram();
    mSceneRenderList.Clear();
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
    gRenderScene.CollectRenderables(mSceneRenderList);

    // todo: sort

    // opaque pass
    
    for (int i = 0; i < mSceneRenderList.mOpaqueElementsCount; ++i)
    {
        const auto& element = mSceneRenderList.mOpaqueElements[i];
        if (element.mAnimatingModel)
        {
            mModelsRenderer.RenderModel(eRenderPass_Opaque, element.mAnimatingModel);
        }
    }

    // translucent pass

    // sort by camera distance, from far to near
    if (mSceneRenderList.mTranslucentElementsCount)
    {
        std::sort(mSceneRenderList.mTranslucentElements, mSceneRenderList.mTranslucentElements + 
            mSceneRenderList.mTranslucentElementsCount, 
            [](const SceneRenderList::ListElement& lhs, const SceneRenderList::ListElement& rhs)
        {
            return lhs.mRenderable->mDistanceToCameraSquared < rhs.mRenderable->mDistanceToCameraSquared;
        });
    }

    for (int i = 0; i < mSceneRenderList.mTranslucentElementsCount; ++i)
    {
        const auto& element = mSceneRenderList.mTranslucentElements[i];
        if (element.mAnimatingModel)
        {
            mModelsRenderer.RenderModel(eRenderPass_Translucent, element.mAnimatingModel);
        }
    }

    mSceneRenderList.Clear();
}

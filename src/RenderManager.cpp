#include "pch.h"
#include "RenderManager.h"
#include "GraphicsDevice.h"
#include "Console.h"
#include "DebugGuiManager.h"
#include "GameMain.h"
#include "ConsoleVariable.h"

//////////////////////////////////////////////////////////////////////////

// cvars

CvarBoolean gCvarRender_DebugDrawEnabled ("r_debugDraw", true, "Enable debug draw", ConsoleVar_Debug | ConsoleVar_Renderer);

//////////////////////////////////////////////////////////////////////////

RenderManager gRenderManager;

//////////////////////////////////////////////////////////////////////////

bool RenderManager::Initialize()
{
    // initialize render programs
    if (!mGuiRenderProgram.LoadProgram())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize render manager");

        Deinit();
        return false;
    }

    if (!mDebugRenderer.Initialize())
    {
        debug_assert(false);
    }

    gGraphicsDevice.SetClearColor(Color32_SkyBlue);

    return true;
}

void RenderManager::Deinit()
{
    mDebugRenderer.Deinit();
    mGuiRenderProgram.FreeProgram();
}

void RenderManager::RenderFrame()
{
    gGraphicsDevice.ClearScreen();

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
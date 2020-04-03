#include "pch.h"
#include "UIRenderer.h"
#include "Console.h"
#include "GraphicsDevice.h"
#include "GraphicsDefs.h"

bool UIRenderer::Initialize()
{
    if (!mUIRenderProgram.LoadProgram())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize ui render manager");

        Deinit();
        return false;
    }
    return true;
}

void UIRenderer::Deinit()
{
    mUIRenderProgram.FreeProgram();
}

void UIRenderer::RenderFrameBegin()
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

    mUIRenderProgram.ActivateProgram();
    mUIRenderProgram.SetViewProjectionMatrix(mTransformationMatrix2D);
    // set scissor box
    gGraphicsDevice.SetScissorRect(screenRect);
}

void UIRenderer::RenderFrameEnd()
{
    mUIRenderProgram.DeactivateProgram();

    // restore scissor box
    Rect2D screenRect;
    screenRect.mX = 0;
    screenRect.mY = 0;
    screenRect.mSizeX = gGraphicsDevice.mViewportRect.mSizeX;
    screenRect.mSizeY = gGraphicsDevice.mViewportRect.mSizeY;
    gGraphicsDevice.SetScissorRect(screenRect);
}
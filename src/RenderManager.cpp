#include "pch.h"
#include "RenderManager.h"
#include "GraphicsDevice.h"

RenderManager gRenderManager;

bool RenderManager::Initialize()
{
    gGraphicsDevice.SetClearColor(Color32_SkyBlue);

    return true;
}

void RenderManager::Deinit()
{
}

void RenderManager::RenderFrame()
{
    gGraphicsDevice.ClearScreen();

    // todo render

    gGraphicsDevice.Present();
}

void RenderManager::ReloadRenderPrograms()
{
}
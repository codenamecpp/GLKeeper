#include "stdafx.h"
#include "TitleScreenView.h"

#include "TextureManager.h"
#include "UiRenderContext.h"

TitleScreenView::TitleScreenView() 
    : UiView(eUiViewLayer_Loadscreen)
{
}

bool TitleScreenView::LoadContent()
{
    if (IsHierarchyLoaded())
        return true;

    TextureManager::LoadParams params;
    params.mConvertNPOT = true;
    params.mInitDefaultOnFail = false;
    mBackgroundTexture = gTextureManager.GetTexture("Titlescreen.png", params);
    return true;
}

void TitleScreenView::Cleanup()
{
    UiView::Cleanup();

    mBackgroundTexture = nullptr;
}

void TitleScreenView::UpdateFrame(float deltaTime)
{

}

void TitleScreenView::RenderFrame(UiRenderContext& renderContext)
{
    const Rect2D& screenRect = renderContext.GetScreenRect();

    renderContext.FillRect(screenRect, COLOR_BLACK);

    if (mBackgroundTexture)
    {
        Rect2D srcRect = mBackgroundTexture->GetImageRect();

        // aspect ratio
        float ratio_w = 1.0f * screenRect.w / srcRect.w;
        float ratio_h = 1.0f * screenRect.h / srcRect.h;
        float ratio = std::min(ratio_h, ratio_w);

        Rect2D dstRect;
        dstRect.w = static_cast<int>((srcRect.w * ratio) + 0.5f);
        dstRect.h = static_cast<int>((srcRect.h * ratio) + 0.5f);
        dstRect.x = screenRect.w / 2 - dstRect.w / 2;
        dstRect.y = 0;

        renderContext.DrawTexture(mBackgroundTexture, COLOR_WHITE, dstRect, srcRect);
    }
}

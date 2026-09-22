#include "stdafx.h"
#include "TitleScreen.h"

#include "TextureManager.h"
#include "UiRenderContext.h"

TitleScreen::TitleScreen() 
    : UiView(eUiViewLayer_Loadscreen)
{
}

bool TitleScreen::LoadContent()
{
    if (IsHierarchyLoaded())
        return true;

    mBackgroundTexture = gTextureManager.GetTexture("Titlescreen.png", eTextureBacking_None);
    return true;
}

void TitleScreen::Cleanup()
{
    UiView::Cleanup();

    mBackgroundTexture = nullptr;
}

void TitleScreen::UpdateFrame(float deltaTime)
{

}

void TitleScreen::RenderFrame(UiRenderContext& renderContext)
{
    const Rect2D& screenRect = renderContext.GetScreenRect();

    renderContext.FillRect(screenRect, COLOR_BLACK);

    if (mBackgroundTexture)
    {
        const TextureRegion& textureRegion = mBackgroundTexture->GetTextureRegion();
        const Rect2D srcRect {0, 0, textureRegion.mRect.w, textureRegion.mRect.h};

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

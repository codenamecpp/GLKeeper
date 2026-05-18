#include "stdafx.h"
#include "LoadingScreenView.h"
#include "TextureManager.h"
#include "UiRenderContext.h"

LoadingScreenView::LoadingScreenView() 
    : UiView(eUiViewLayer_Loadscreen)
{
}

void LoadingScreenView::StartLoading()
{
    if (!Activate())
    {
        cxx_assert(false);        
    }

    mLoadingProgress = 0.0f;
}

void LoadingScreenView::FinishLoading()
{
    Deactivate();

    mLoadingProgress = 0.0f;
}

void LoadingScreenView::UpdateLoadingProgress(float progress)
{
    mLoadingProgress = std::clamp(progress, 0.0f, 1.0f);
}

bool LoadingScreenView::LoadContent()
{
    if (IsHierarchyLoaded())
        return true;

    TextureManager::LoadParams params;
    params.mConvertNPOT = true;
    params.mInitDefaultOnFail = false;
    mBackgroundTexture = gTextureManager.GetTexture("LoadingScreen800x600.444", params);

    mProgressBarSrcRect = {30, 556, 206, 16};
    mProgressBarFillColor = Color32{244, 113, 0, 255};

    return true;
}

void LoadingScreenView::Cleanup()
{
    UiView::Cleanup();

    mBackgroundTexture = nullptr;
}

void LoadingScreenView::UpdateFrame(float deltaTime)
{

}

void LoadingScreenView::RenderFrame(UiRenderContext& renderContext)
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

        if (mLoadingProgress > 0.0f)
        {
            Rect2D fillRect {
                dstRect.x + static_cast<int>((mProgressBarSrcRect.x * ratio) + 0.5f),
                dstRect.y + static_cast<int>((mProgressBarSrcRect.y * ratio) + 0.5f),
                static_cast<int>((mProgressBarSrcRect.w * ratio) + 0.5f),
                static_cast<int>((mProgressBarSrcRect.h * ratio) + 0.5f)
            };
            fillRect.w = static_cast<int>(fillRect.w * mLoadingProgress);
            renderContext.FillRect(fillRect, mProgressBarFillColor);
        }
    }
}

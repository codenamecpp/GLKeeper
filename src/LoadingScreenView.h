#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"

//////////////////////////////////////////////////////////////////////////

// Loading level ui screen
class LoadingScreenView: public UiView
{
public:
    LoadingScreenView();

    void StartLoading();
    void FinishLoading();
    void UpdateLoadingProgress(float progress);

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void UpdateFrame(float deltaTime) override;
    void RenderFrame(UiRenderContext& renderContext) override;

private:
    Texture* mBackgroundTexture = nullptr;

    Rect2D mProgressBarSrcRect;
    Color32 mProgressBarFillColor;

    float mLoadingProgress = 0.0f;
};

//////////////////////////////////////////////////////////////////////////
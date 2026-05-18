#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"

//////////////////////////////////////////////////////////////////////////

// game title screen
class TitleScreenView: public UiView
{
public:
    TitleScreenView();

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void UpdateFrame(float deltaTime) override;
    void RenderFrame(UiRenderContext& renderContext) override;

private:
    Texture* mBackgroundTexture = nullptr;
};

//////////////////////////////////////////////////////////////////////////
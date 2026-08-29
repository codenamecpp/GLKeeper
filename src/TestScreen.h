#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"

//////////////////////////////////////////////////////////////////////////

class TestScreen: public UiView, public UiEventListener
{
public:
    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void UpdateFrame(float deltaTime) override;
    void RenderFrame(UiRenderContext& renderContext) override;
    void InputEvent(KeyInputEvent& inputEvent) override;

    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
};

//////////////////////////////////////////////////////////////////////////
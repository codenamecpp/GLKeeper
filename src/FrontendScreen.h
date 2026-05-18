#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"
#include "GameSessionDefs.h"

//////////////////////////////////////////////////////////////////////////

class FrontendScreen: public UiView, public UiEventListener
{
public:
    FrontendScreen(FrontendController& frontend);

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void UpdateFrame(float deltaTime) override;

private:
    // override UiView
    void OnActivated() override;
    void OnDeactivated() override;

    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEventDesc* eventDesc) override;

private:
    FrontendController& mFrontend;
};

//////////////////////////////////////////////////////////////////////////
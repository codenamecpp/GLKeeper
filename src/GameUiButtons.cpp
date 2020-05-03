#include "pch.h"
#include "GameUiButtons.h"
#include "GuiButton.h"
#include "GuiWidget.h"
#include "GuiEvent.h"

// feature button custom events
static cxx::unique_string EventId_FeatureButtonSetupState("init");

//////////////////////////////////////////////////////////////////////////

GameUiFeatureButton::GameUiFeatureButton()
    : mActiveState()
    , mAvailableState()
{
}

GameUiFeatureButton::~GameUiFeatureButton()
{
    Unbind();
}

void GameUiFeatureButton::Bind(GuiButton* button)
{
    if (button == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (mBoundButton == button)
        return;

    Unbind();

    mBoundButton = button;
    mBoundButton->SetActionsContext(this);
    mBoundButton->mActions.EmitEvent(GuiEvent::CustomEvent(nullptr, EventId_FeatureButtonSetupState));
}

void GameUiFeatureButton::Unbind()
{
    if (mBoundButton == nullptr)
        return;

    debug_assert(mBoundButton->mActionsContext == this);

    mBoundButton->SetActionsContext(nullptr);
    mBoundButton = nullptr;
}

void GameUiFeatureButton::SetActiveState(bool isActive)
{
    if (mActiveState == isActive)
        return;

    mActiveState = isActive;
    if (mBoundButton)
    {
        mBoundButton->mActions.EmitEvent(GuiEvent::CustomEvent(nullptr, EventId_FeatureButtonSetupState));
    }
}

void GameUiFeatureButton::SetAvailableState(bool isAvailable)
{
    if (mAvailableState == isAvailable)
        return;

    mAvailableState = isAvailable;
    if (mBoundButton)
    {
        mBoundButton->mActions.EmitEvent(GuiEvent::CustomEvent(nullptr, EventId_FeatureButtonSetupState));
    }
}

bool GameUiFeatureButton::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    static cxx::unique_string id_active("active");
    static cxx::unique_string id_available("available");

    if (name == id_active)
    {
        isTrue = IsActiveState();
        return true;
    }
    if (name == id_available)
    {
        isTrue = IsAvailableState();
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
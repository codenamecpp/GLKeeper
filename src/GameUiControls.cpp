#include "pch.h"
#include "GameUiControls.h"
#include "GuiButton.h"
#include "GuiWidget.h"
#include "GuiEvent.h"
#include "GuiHelpers.h"
#include "GuiPictureBox.h"

//////////////////////////////////////////////////////////////////////////

GameUiFeatureButton::~GameUiFeatureButton()
{
    SetControl(nullptr);
}

void GameUiFeatureButton::SetControl(GuiWidget* button)
{
    if (mControl == button)
        return;

    // unbind
    if (mControl)
    {
        debug_assert(mControl->mActionsContext == this);

        mControl->SetActionsContext(nullptr);
        mControl = nullptr;
        mIcon = nullptr;
        mActiveOverlay = nullptr;
    }

    if (button == nullptr)
        return;

    // bind
    mControl = button;
    mControl->SetActionsContext(this);
    mControl->SetVisible(true);
    mControl->SetEnabled(true);
    UpdateState();

    // get textures
    if (GuiPictureBox* pictureBox = GetPictureBox("icon"))
    {
        mIcon = pictureBox->mTexture;
    }
    debug_assert(mIcon);

    if (GuiPictureBox* pictureBox = GetPictureBox("active_overlay"))
    {
        mActiveOverlay = pictureBox->mTexture;
    }
    debug_assert(mActiveOverlay);
}

void GameUiFeatureButton::SetActiveState(bool isActive)
{
    if (mActiveState == isActive)
        return;

    mActiveState = isActive;
    UpdateState();
}

void GameUiFeatureButton::SetAvailableState(bool isAvailable)
{
    if (mAvailableState == isAvailable)
        return;

    mAvailableState = isAvailable;
    UpdateState();
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

void GameUiFeatureButton::UpdateState()
{
    static cxx::unique_string id_FeatureButtonSetupState("init");
    if (mControl)
    {
        GuiEvent eventData(nullptr, id_FeatureButtonSetupState);
        mControl->mActions.EmitEvent(eventData);
    }
}

GuiPictureBox* GameUiFeatureButton::GetPictureBox(const std::string& name) const
{
    if (mControl == nullptr)
        return nullptr;

    if (GuiWidget* target = mControl->SearchForChild("icon"))
    {
        GuiPictureBox* pictureBox = GuiCastWidgetClass<GuiPictureBox>(target);
        return pictureBox;
    }
    return nullptr;
}

void GameUiFeatureButton::SetIcon(Texture2D* texture)
{
    if (mIcon == texture)
        return;

    if (GuiPictureBox* pictureBox = GetPictureBox("icon"))
    {
        pictureBox->SetTexture(texture);
        mIcon = pictureBox->mTexture;
        return;
    }
    debug_assert(false);
}

void GameUiFeatureButton::SetActiveOverlay(Texture2D* texture)
{
    if (mActiveOverlay == texture)
        return;

    if (GuiPictureBox* pictureBox = GetPictureBox("active_overlay"))
    {
        pictureBox->SetTexture(texture);
        mActiveOverlay = pictureBox->mTexture;
        return;
    }
    debug_assert(false);
}

//////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "GameUiControls.h"
#include "GuiButton.h"
#include "GuiWidget.h"
#include "GuiEvent.h"
#include "GuiHelpers.h"
#include "GuiPictureBox.h"

//////////////////////////////////////////////////////////////////////////

const cxx::unique_string GameUiCtlPanelIconTemplateId("ctl_panel_icon_template");

//////////////////////////////////////////////////////////////////////////

GameUiCtlPanelIcon::~GameUiCtlPanelIcon()
{
    SetControl(nullptr);
}

void GameUiCtlPanelIcon::SetControl(GuiWidget* button)
{
    if (mControl == button)
        return;

    // unbind
    if (mControl)
    {
        debug_assert(mControl->mActionsContext == this);

        mControl->SetActionsContext(nullptr);
        mControl = nullptr;
        mPicture = nullptr;
        mActiveOverlay = nullptr;
    }

    if (button == nullptr)
        return;

    debug_assert(button->mTemplateClassName == GameUiCtlPanelIconTemplateId);

    // bind
    mControl = button;
    mControl->SetActionsContext(this);
    mControl->SetVisible(true);
    mControl->SetEnabled(true);
    UpdateState();

    // get textures
    if (GuiPictureBox* pictureBox = GetPictureBox("icon"))
    {
        mPicture = pictureBox->mTexture;
    }
    debug_assert(mPicture);

    if (GuiPictureBox* pictureBox = GetPictureBox("active_overlay"))
    {
        mActiveOverlay = pictureBox->mTexture;
    }
    debug_assert(mActiveOverlay);
}

void GameUiCtlPanelIcon::SetActiveState(bool isActive)
{
    if (mActiveState == isActive)
        return;

    mActiveState = isActive;
    UpdateState();
}

void GameUiCtlPanelIcon::SetAvailableState(bool isAvailable)
{
    if (mAvailableState == isAvailable)
        return;

    mAvailableState = isAvailable;
    UpdateState();
}

bool GameUiCtlPanelIcon::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
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

void GameUiCtlPanelIcon::UpdateState()
{
    static cxx::unique_string id_FeatureButtonSetupState("on_change_state");
    if (mControl)
    {
        GuiEvent eventData(nullptr, id_FeatureButtonSetupState);
        mControl->mActions.EmitEvent(eventData);
    }
}

GuiPictureBox* GameUiCtlPanelIcon::GetPictureBox(const std::string& name) const
{
    if (mControl == nullptr)
        return nullptr;

    if (GuiWidget* target = mControl->SearchForChild(name))
    {
        GuiPictureBox* pictureBox = GuiCastWidgetClass<GuiPictureBox>(target);
        return pictureBox;
    }
    return nullptr;
}

void GameUiCtlPanelIcon::SetPicture(Texture2D* texture)
{
    if (mPicture == texture)
        return;

    if (GuiPictureBox* pictureBox = GetPictureBox("icon"))
    {
        pictureBox->SetTexture(texture);
        mPicture = pictureBox->mTexture;
        return;
    }
    debug_assert(false);
}

void GameUiCtlPanelIcon::SetActiveOverlay(Texture2D* texture)
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
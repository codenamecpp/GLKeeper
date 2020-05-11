#include "pch.h"
#include "UiCtlPanelIcon.h"
#include "GuiButton.h"
#include "GuiWidget.h"
#include "GuiEvent.h"
#include "GuiHelpers.h"
#include "GuiPictureBox.h"
#include "TexturesManager.h"
#include "ScenarioDefs.h"

const cxx::unique_string UiCtlPanelIconTemplateId("ctl_panel_icon_template");

UiCtlPanelIcon::UiCtlPanelIcon(GuiWidget* control)
    : mControl(control)
{
    debug_assert(mControl);
    if (mControl == nullptr)
        return;

    debug_assert(mControl->mTemplateClassName == UiCtlPanelIconTemplateId);

    // bind
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

UiCtlPanelIcon::~UiCtlPanelIcon()
{
    if (mControl) // unbind
    {
        debug_assert(mControl->mActionsContext == this);

        mControl->SetActionsContext(nullptr);
        mControl = nullptr;
        mPicture = nullptr;
        mActiveOverlay = nullptr;
    }
}

void UiCtlPanelIcon::SetNullContext()
{
    mRoomDefinition = nullptr;
    mObjectDefinition = nullptr;
}

void UiCtlPanelIcon::SetContext(RoomDefinition* roomDefinition)
{
    SetNullContext();
    mRoomDefinition = roomDefinition;
    if (mRoomDefinition && 
        mRoomDefinition->mGuiIconResource.IsDefined())
    {
        Texture2D* texture = gTexturesManager.GetTexture2D(mRoomDefinition->mGuiIconResource.mResourceName);
        SetPicture(texture);
    }
    debug_assert(mRoomDefinition);
}

void UiCtlPanelIcon::SetContext(GameObjectDefinition* objectDefinition)
{
    SetNullContext();
    mObjectDefinition = objectDefinition;
    if (mObjectDefinition && 
        mObjectDefinition->mGuiIconResource.IsDefined())
    {
        Texture2D* texture = gTexturesManager.GetTexture2D(mObjectDefinition->mGuiIconResource.mResourceName);
        SetPicture(texture);
    }
    debug_assert(mObjectDefinition);
}

void UiCtlPanelIcon::SetActiveState(bool isActive)
{
    if (mActiveState == isActive)
        return;

    mActiveState = isActive;
    UpdateState();
}

void UiCtlPanelIcon::SetAvailableState(bool isAvailable)
{
    if (mAvailableState == isAvailable)
        return;

    mAvailableState = isAvailable;
    UpdateState();
}

bool UiCtlPanelIcon::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
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

void UiCtlPanelIcon::UpdateState()
{
    static const cxx::unique_string id_change_state("on_change_state");
    if (mControl)
    {
        GuiEvent eventData(mControl, id_change_state);
        mControl->mActions.EmitEvent(eventData);
    }
}

GuiPictureBox* UiCtlPanelIcon::GetPictureBox(const std::string& name) const
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

void UiCtlPanelIcon::SetPicture(Texture2D* texture)
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

void UiCtlPanelIcon::SetActiveOverlay(Texture2D* texture)
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

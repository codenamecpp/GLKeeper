#pragma once

#include "GuiDefs.h"
#include "GuiAction.h"
#include "UserInterfaceDefs.h"

// control panel icon button
// used for selecting rooms, spells, objects etc
class UiCtlPanelIcon final: public cxx::noncopyable
    , public GuiActionContext
{
public:
    // readonly
    RoomDefinition* mRoomDefinition = nullptr;
    GameObjectDefinition* mObjectDefinition = nullptr;

    GuiWidget* mControl;
    Texture2D* mPicture = nullptr;
    Texture2D* mActiveOverlay = nullptr;

public:
    // bind controller to specific widget
    UiCtlPanelIcon(GuiWidget* control);
    ~UiCtlPanelIcon();

    // setup icon context
    void SetNullContext();
    void SetContext(RoomDefinition* roomDefinition);
    void SetContext(GameObjectDefinition* objectDefinition);

    // set current active state
    void SetActiveState(bool isActive);
    void ToggleActiveState()
    {
        SetActiveState(!IsActiveState());
    }
    bool IsActiveState() const { return mActiveState; }
    // set current available state
    void SetAvailableState(bool isAvailable);
    void ToggleAvailableState()
    {
        SetAvailableState(!IsAvailableState());
    }
    bool IsAvailableState() const { return mAvailableState; }
    // set picture and overlay textures
    void SetPicture(Texture2D* texture);
    void SetActiveOverlay(Texture2D* texture);

private:
    // override GuiActionContext
    bool ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue) override;

    void UpdateState();
    // get child picture widget
    GuiPictureBox* GetPictureBox(const std::string& name) const;

private:
    bool mAvailableState = true;
    bool mActiveState = false;
};
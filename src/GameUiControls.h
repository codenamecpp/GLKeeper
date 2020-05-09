#pragma once

#include "GuiDefs.h"
#include "GuiAction.h"

//////////////////////////////////////////////////////////////////////////

extern const cxx::unique_string GameUiCtlPanelIconTemplateId;

//////////////////////////////////////////////////////////////////////////

// control panel button
// used for selecting rooms, spells, objects etc
class GameUiCtlPanelIcon final: public GuiActionContext
{
public:
    // readonly
    GuiWidget* mControl = nullptr;
    Texture2D* mPicture = nullptr;
    Texture2D* mActiveOverlay = nullptr;

public:
    ~GameUiCtlPanelIcon();
    // bind controller to specific widget or unbind if null
    void SetControl(GuiWidget* button);

    // set current active state
    void SetActiveState(bool isActive);
    bool IsActiveState() const { return mActiveState; }
    void ToggleActiveState()
    {
        SetActiveState(!IsActiveState());
    }

    // set current available state
    void SetAvailableState(bool isAvailable);
    bool IsAvailableState() const { return mAvailableState; }
    void ToggleAvailableState()
    {
        SetAvailableState(!IsAvailableState());
    }
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
    // extended states
    bool mAvailableState = true;
    bool mActiveState = false;
};

//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GuiDefs.h"
#include "GuiAction.h"

//////////////////////////////////////////////////////////////////////////

// facture button
// used in hud control panel for selecting rooms, spells, objects etc
class GameUiFeatureButton: public GuiActionContext
{
public:
    // readonly
    GuiWidget* mControl = nullptr;
    Texture2D* mIcon = nullptr;
    Texture2D* mActiveOverlay = nullptr;

public:
    GameUiFeatureButton() = default;
    ~GameUiFeatureButton();

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
    // set icon and overlay textures
    void SetIcon(Texture2D* texture);
    void SetActiveOverlay(Texture2D* texture);

private:
    // override GuiActionContext
    bool ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue) override;

    void UpdateState();

    // get child picture widget
    GuiPictureBox* GetPictureBox(const std::string& name) const;
private:
    // extended states
    bool mActiveState = false;
    bool mAvailableState = false;
};

//////////////////////////////////////////////////////////////////////////
#pragma once

#include "GuiDefs.h"
#include "GuiAction.h"

//////////////////////////////////////////////////////////////////////////

// facture button
// used in hud control panel for selecting rooms, spells, objects etc
class GameUiFeatureButton: public GuiActionContext
{
public:
    GameUiFeatureButton();
    ~GameUiFeatureButton();

    // bind controller to specific button
    void Bind(GuiButton* button);
    void Unbind();

    // active state
    void SetActiveState(bool isActive);
    bool IsActiveState() const { return mActiveState; }
    void ToggleActiveState()
    {
        SetActiveState(!IsActiveState());
    }
    // availability state
    void SetAvailableState(bool isAvailable);
    bool IsAvailableState() const { return mAvailableState; }
    void ToggleAvailableState()
    {
        SetAvailableState(!IsAvailableState());
    }
private:
    // override GuiActionContext
    bool ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue) override;

private:
    GuiButton* mBoundButton = nullptr;

    // extended states
    bool mActiveState;
    bool mAvailableState;
};

//////////////////////////////////////////////////////////////////////////
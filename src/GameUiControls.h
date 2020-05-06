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

    // bind controller to specific button
    void SetControl(GuiWidget* button);
    void SetNull();
    // test whether controller is bound to gui button
    bool NonNull() const;

    // set icon and overlay textures
    // @param textureName, texture: Specific texture
    void SetIcon(const std::string& textureName);
    void SetIcon(Texture2D* texture);

    // @param textureName, texture: Specific texture
    void SetActiveOverlay(const std::string& textureName);
    void SetActiveOverlay(Texture2D* texture);

    // set active state
    void SetActiveState(bool isActive);
    bool IsActiveState() const { return mActiveState; }
    void ToggleActiveState()
    {
        SetActiveState(!IsActiveState());
    }
    // set availability state
    void SetAvailableState(bool isAvailable);
    bool IsAvailableState() const { return mAvailableState; }
    void ToggleAvailableState()
    {
        SetAvailableState(!IsAvailableState());
    }
    // show or hide button
    void SetVisible(bool isShown);
    bool IsVisible() const;

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
#pragma once

#include "GenericGamestate.h"
#include "GodmodeCameraController.h"
#include "GameplayGameScreen.h"
#include "MapInteractionController.h"

// in-game gamestate
class GameplayGamestate: public GenericGamestate
{
public:
    GameplayGameScreen mGameplayGameScreen;
    MapInteractionController mMapInteractionControl;

public:
    // enter game state
    void HandleGamestateEnter() override;

    // leave game state
    void HandleGamestateLeave() override;

    // process game state frame logic
    // @param theDeltaTime: Time since previous frame
    void HandleUpdateFrame() override;

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleInputEvent(MouseMovedInputEvent& inputEvent) override;
    void HandleInputEvent(MouseScrollInputEvent& inputEvent) override;
    void HandleInputEvent(KeyInputEvent& inputEvent) override;
    void HandleInputEvent(KeyCharEvent& inputEvent) override;

    void HandleScreenResolutionChanged() override;
    
    // test whether map interaction is active
    bool IsMapInteractionActive() const;

private:
    GodmodeCameraController mGodmodeCameraControl;
};
#pragma once

#include "GenericGamestate.h"
#include "GuiTestScreen.h"

class GuiTestGamestate: public GenericGamestate
{
public:
    GuiTestGamestate();

    void HandleGamestateEnter() override;
    void HandleGamestateLeave() override;
    void HandleUpdateFrame() override;

    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleInputEvent(MouseMovedInputEvent& inputEvent) override;
    void HandleInputEvent(MouseScrollInputEvent& inputEvent) override;
    void HandleInputEvent(KeyInputEvent& inputEvent) override;
    void HandleInputEvent(KeyCharEvent& inputEvent) override;

public:
    GuiTestScreen mScreen;
};
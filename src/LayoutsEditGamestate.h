#pragma once

#include "GenericGamestate.h"
#include "ToolsUILayoutsEditWindow.h"

class LayoutsEditGamestate: public GenericGamestate
{
public:
    void HandleGamestateEnter() override;
    void HandleGamestateLeave() override;
    void HandleUpdateFrame() override;

    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleInputEvent(MouseMovedInputEvent& inputEvent) override;
    void HandleInputEvent(MouseScrollInputEvent& inputEvent) override;
    void HandleInputEvent(KeyInputEvent& inputEvent) override;
    void HandleInputEvent(KeyCharEvent& inputEvent) override;

private:
    ToolsUILayoutsEditWindow mLayoutsEditWindow;
};
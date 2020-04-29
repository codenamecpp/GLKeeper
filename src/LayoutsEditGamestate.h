#pragma once

#include "GenericGamestate.h"
#include "ToolsUILayoutsEditWindow.h"
#include "GuiHierarchy.h"
#include "GuiEvent.h"
#include "GuiHelpers.h"

class LayoutsEditGamestate: public GenericGamestate
    , public GuiEventsHandler
{
public:
    LayoutsEditGamestate();

    void HandleGamestateEnter() override;
    void HandleGamestateLeave() override;
    void HandleUpdateFrame() override;

    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleInputEvent(MouseMovedInputEvent& inputEvent) override;
    void HandleInputEvent(MouseScrollInputEvent& inputEvent) override;
    void HandleInputEvent(KeyInputEvent& inputEvent) override;
    void HandleInputEvent(KeyCharEvent& inputEvent) override;

protected:
    // override GuiEventsHandler
    void HandleClick(GuiWidget* sender) override;
    void HandleMouseEnter(GuiWidget* sender) override;
    void HandleMouseLeave(GuiWidget* sender) override;
    void HandleMouseDown(GuiWidget* sender, eMouseButton mbutton) override;
    void HandleMouseUp(GuiWidget* sender, eMouseButton mbutton) override;
    void HandleEvent(GuiWidget* sender, cxx::unique_string eventId) override;

    void SetupVisibility(GuiWidget* widget);

private:
    ToolsUILayoutsEditWindow mLayoutsEditWindow;
    GuiHierarchy mHier;
};
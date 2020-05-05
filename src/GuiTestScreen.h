#pragma once

#include "GuiScreen.h"
#include "GuiHierarchy.h"
#include "GuiEvent.h"
#include "GuiHelpers.h"
#include "GameUiButtons.h"

class GuiTestScreen: public GuiScreen
    , public GuiEventsHandler
    , public GuiActionContext
{
public:
    bool ReloadScreen();

private:
    // override GuiScreen
    void HandleRenderScreen(GuiRenderer& renderContext) override;
    void HandleUpdateScreen() override;
    void HandleCleanupScreen() override;
    bool HandleInitializeScreen() override;
    void HandleStartScreen() override;
    void HandleEndScreen() override;

    // override GuiEventsHandler
    void HandleClick(GuiWidget* sender, eMouseButton mbuton) override;
    void HandlePressStart(GuiWidget* sender, eMouseButton mbuton) override;
    void HandlePressEnd(GuiWidget* sender, eMouseButton mbuton) override;
    void HandleMouseEnter(GuiWidget* sender) override;
    void HandleMouseLeave(GuiWidget* sender) override;
    void HandleMouseDown(GuiWidget* sender, eMouseButton mbutton) override;
    void HandleMouseUp(GuiWidget* sender, eMouseButton mbutton) override;
    void HandleEvent(GuiWidget* sender, cxx::unique_string eventId) override;

    // override GuiActionContext
    bool ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue) override;

private:
    GameUiFeatureButton mFeatureButtonController;
};
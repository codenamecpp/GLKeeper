#pragma once

#include "GuiScreen.h"
#include "GuiHierarchy.h"
#include "GuiEvent.h"
#include "GuiHelpers.h"
#include "GameUiControls.h"

class GuiTestScreen: public GuiScreen
{
public:
    GuiTestScreen();
    bool ReloadScreen();

private:
    // override GuiScreen
    void HandleScreenRender(GuiRenderer& renderContext) override;
    void HandleScreenUpdate() override;
    void HandleScreenCleanup() override;
    void HandleScreenLoad() override;
    void HandleScreenShow() override;
    void HandleScreenHide() override;

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
    std::vector<GameUiFeatureButton> mFeatureButtons;
};
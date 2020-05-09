#pragma once

#include "GuiScreen.h"
#include "GameUiControls.h"
#include "UserInterfaceDefs.h"

class GameplayGameScreen: public GuiScreen
{
public:
    GameplayGameScreen();

    // select control panel tab and setup its content
    // @param ctlPanelTab: Tab identifier
    void SelectCtlPanel(eGameUiCtlPanel ctlPanelTab);

    // synchronize hud controls with current logic state
    void UpdateUserInterfaceState();
    
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

    void SetupCurrentCtlPanelContent();

private:
    
    GuiWidget* mCtlPanelIconsContainer = nullptr;

    // control panel bar
    eGameUiCtlPanel mCurrentCtlPanel = eGameUiCtlPanel_Rooms;
    std::vector<GameUiCtlPanelIcon> mCtlPanelTabsIcons;
    std::vector<GameUiCtlPanelIcon> mCtlPanelIcons;
    GameUiCtlPanelIcon mCtlPanelIconSell;
    GameUiCtlPanelIcon mCtlPanelIconDig;
};

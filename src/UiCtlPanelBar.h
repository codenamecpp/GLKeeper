#pragma once

#include "GuiDefs.h"
#include "GuiAction.h"
#include "UserInterfaceDefs.h"
#include "GuiEvent.h"

// control panel bar
class UiCtlPanelBar: public cxx::noncopyable
    , public GuiActionContext
    , public GuiEventsHandler
{
public:
    // readonly
    GuiWidget* mControl;

public:
    // bind controller to specific widget
    UiCtlPanelBar(GuiWidget* control);
    ~UiCtlPanelBar();

    // select control panel tab and setup its content
    // @param panel: Panel identifier
    void SelectPanel(eUiCtlPanelId panel);

private:
    // override GuiActionContext
    bool ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue) override;

    // override GuiEventsHandler
    void HandlePressStart(GuiWidget* sender, eMouseButton mbutton) override;
    void HandlePressEnd(GuiWidget* sender, eMouseButton mbutton) override;

    void BindControls();
    void SetupCurrentPanelContent();

    void UnusePanelIcons();
    UiCtlPanelIcon* UsePanelIcon();

private:
    GuiWidget* mPanelIconsContainer = nullptr;
    eUiCtlPanelId mCurrentPanel = eUiCtlPanelId_Rooms;
    UiCtlPanelIcon* mIconCreatures = nullptr;
    UiCtlPanelIcon* mIconRooms = nullptr;
    UiCtlPanelIcon* mIconTraps = nullptr;
    UiCtlPanelIcon* mIconSpells = nullptr;
    UiCtlPanelIcon* mIconSell = nullptr;
    UiCtlPanelIcon* mIconDig = nullptr;
    std::vector<UiCtlPanelIcon*> mPanelIcons;
};
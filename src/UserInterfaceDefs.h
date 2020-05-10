#pragma once

// forwards
class UiCtlPanelBar;
class UiCtlPanelIcon;

// control panel identifier
enum eUiCtlPanelId
{
    eUiCtlPanelId_Creatures,
    eUiCtlPanelId_Rooms,
    eUiCtlPanelId_Spells,
    eUiCtlPanelId_Traps,
    eUiCtlPanelId_COUNT
};

// template widget identifiers
extern const cxx::unique_string UiCtlPanelIconTemplateId;
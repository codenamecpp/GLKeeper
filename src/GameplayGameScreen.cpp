#include "pch.h"
#include "GameplayGameScreen.h"
#include "GameWorld.h"
#include "ScenarioDefs.h"
#include "TexturesManager.h"

GameplayGameScreen::GameplayGameScreen() 
    : GuiScreen(cxx::unique_string("gameplay_screen"))
{
}

void GameplayGameScreen::SelectCtlPanel(eGameUiCtlPanel ctlPanelTab)
{
    if (mCurrentCtlPanel == ctlPanelTab)
        return;

    mCurrentCtlPanel = ctlPanelTab;
    SetupCurrentCtlPanelContent();
}

void GameplayGameScreen::UpdateUserInterfaceState()
{

}

void GameplayGameScreen::HandleScreenRender(GuiRenderer& renderContext)
{
}

void GameplayGameScreen::HandleScreenUpdate()
{
}

void GameplayGameScreen::HandleScreenCleanup()
{
    mCtlPanelIconsContainer = nullptr;
    mCtlPanelTabsIcons.clear();
    mCtlPanelIcons.clear();
    mCtlPanelIconSell.SetControl(nullptr);
    mCtlPanelIconDig.SetControl(nullptr);
}

void GameplayGameScreen::HandleScreenLoad()
{
    // init ctl panel bar

    mCtlPanelIconsContainer = mHier.SearchForWidget("control_panel_content");
    debug_assert(mCtlPanelIconsContainer);

    // bind tabs buttons
    mCtlPanelTabsIcons.resize(eGameUiCtlPanel_COUNT);
    {
        GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_creatures");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mCtlPanelTabsIcons[eGameUiCtlPanel_Creatures].SetControl(control);
    }
    {
        GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_rooms");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mCtlPanelTabsIcons[eGameUiCtlPanel_Rooms].SetControl(control);
    }
    {
        GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_spells");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mCtlPanelTabsIcons[eGameUiCtlPanel_Spells].SetControl(control);
    }
    {
        GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_traps");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mCtlPanelTabsIcons[eGameUiCtlPanel_Traps].SetControl(control);
    }

    // bind sell and dig buttons
    {
        GuiWidget* control = mHier.SearchForWidget("ctl_panel_sell_button");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mCtlPanelIconSell.SetControl(control);
    }
    {
        GuiWidget* control = mHier.SearchForWidget("ctl_panel_dig_button");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mCtlPanelIconDig.SetControl(control);
    }
}

void GameplayGameScreen::HandleScreenShow()
{
    SetupCurrentCtlPanelContent();
}

void GameplayGameScreen::HandleScreenHide()
{
}

void GameplayGameScreen::HandleClick(GuiWidget* sender, eMouseButton mbuton)
{
}

void GameplayGameScreen::HandlePressStart(GuiWidget* sender, eMouseButton mbuton)
{
    bool isPrimatyFunction = (mbuton == eMouseButton_Left);
    bool isSecondaryFunction = (mbuton == eMouseButton_Right);

    // identify event source

    if (sender == mCtlPanelIconDig.mControl)
    {
        return;
    }
    if (sender == mCtlPanelIconSell.mControl)
    {
        return;
    }
    // panel tab ?
    for (int icurr = 0; icurr < eGameUiCtlPanel_COUNT; ++icurr)
    {
        if (sender == mCtlPanelTabsIcons[icurr].mControl)
        {
            if (isPrimatyFunction)
            {
                eGameUiCtlPanel ctlpanel = (eGameUiCtlPanel) icurr;
                SelectCtlPanel(ctlpanel);
            }
            return;
        }
    }
    // panel icon ?
    for (const GameUiCtlPanelIcon& currButton: mCtlPanelIcons)
    {
        if (sender == currButton.mControl)
        {
            return;
        }
    }
}

void GameplayGameScreen::HandlePressEnd(GuiWidget* sender, eMouseButton mbuton)
{
}

void GameplayGameScreen::HandleMouseEnter(GuiWidget* sender)
{
}

void GameplayGameScreen::HandleMouseLeave(GuiWidget* sender)
{
}

void GameplayGameScreen::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
{
}

void GameplayGameScreen::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
}

void GameplayGameScreen::HandleEvent(GuiWidget* sender, cxx::unique_string eventId)
{
}

bool GameplayGameScreen::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    return false;
}

void GameplayGameScreen::SetupCurrentCtlPanelContent()
{
    if (mCtlPanelIconsContainer == nullptr)
    {
        debug_assert(false);
        return;
    }

    // hide all buttons
    for (GameUiCtlPanelIcon& currButton: mCtlPanelIcons)
    {
        currButton.mControl->SetVisible(false);
    }

    mCtlPanelIcons.clear();

    // init rooms buttons
    if (mCurrentCtlPanel == eGameUiCtlPanel_Rooms)
    {
        std::vector<RoomDefinition*> availableRooms;
        availableRooms.reserve(gGameWorld.mScenarioData.mRoomDefs.size());
        for (int iroom = 0, numRooms = gGameWorld.mScenarioData.mRoomDefs.size(); iroom < numRooms; ++iroom)
        {
            RoomDefinition* roomDef = &gGameWorld.mScenarioData.mRoomDefs[iroom];
            // todo : rooms availability to player
            if (roomDef->mBuildable)
            {
                availableRooms.push_back(roomDef);
            }
        }
        // sort in order
        std::sort(availableRooms.begin(), availableRooms.end(), 
            [](RoomDefinition* lhs, RoomDefinition* rhs)
            {
                return lhs->mOrderInEditor < rhs->mOrderInEditor;
            });

        mCtlPanelIcons.resize(availableRooms.size());
        // bind buttons
        for (size_t icurr = 0; icurr < availableRooms.size(); ++icurr)
        {
            GuiWidget* currControl = mCtlPanelIconsContainer->GetChild(icurr);
            if (currControl == nullptr)
            {
                currControl = mHier.CreateTemplateWidget(GameUiCtlPanelIconTemplateId);
                debug_assert(currControl);
                mCtlPanelIconsContainer->AttachChild(currControl);
            }
            debug_assert(currControl);
            currControl->SetVisible(true);

            RoomDefinition* currRoomDef = availableRooms[icurr];
            mCtlPanelIcons[icurr].SetControl(currControl);
            Subscribe(GuiEventId_OnPressStart, currControl);

            Texture2D* roomIcon = gTexturesManager.GetTexture2D(currRoomDef->mGuiIcon.mResourceName);
            mCtlPanelIcons[icurr].SetPicture(roomIcon);
        }
    }
    
    // set overlay texture same as tab overlay
    for (size_t icurr = 0; icurr < mCtlPanelIcons.size(); ++icurr)
    {
        Texture2D* overlayTexture = mCtlPanelTabsIcons[mCurrentCtlPanel].mActiveOverlay;
        mCtlPanelIcons[icurr].SetActiveOverlay(overlayTexture);
    }

    UpdateUserInterfaceState();
}

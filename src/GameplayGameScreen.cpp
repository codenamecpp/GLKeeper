#include "pch.h"
#include "GameplayGameScreen.h"
#include "GameWorld.h"
#include "ScenarioDefs.h"
#include "TexturesManager.h"

GameplayGameScreen::GameplayGameScreen() 
    : GuiScreen(cxx::unique_string("gameplay_screen"))
{
}

void GameplayGameScreen::UpdateUserInterfaceState()
{

}

void GameplayGameScreen::ReloadScreen()
{
    CleanupScreen();

    bool isSuccess = ShowScreen();
    debug_assert(isSuccess);
}

void GameplayGameScreen::HandleScreenRender(GuiRenderer& renderContext)
{
}

void GameplayGameScreen::HandleScreenUpdate()
{
}

void GameplayGameScreen::HandleScreenCleanup()
{
}

void GameplayGameScreen::HandleScreenLoad()
{
}

void GameplayGameScreen::HandleScreenShow()
{
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

#include "pch.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "Console.h"
#include "TimeManager.h"
#include "GuiPictureBox.h"
#include "InputsManager.h"
#include "GuiButton.h"
#include "GuiPanel.h"
#include "GuiScrollbar.h"
#include "GuiHierarchy.h"
#include "GraphicsDevice.h"
#include "FileSystem.h"
#include "GuiScreen.h"

GuiManager gGuiManager;

GuiManager::GuiManager()
{
    mEventsQueue.reserve(256);
    mProcessingEventsQueue.reserve(256);
}

bool GuiManager::Initialize()
{
    RegisterWidgetsClasses();
    return true;
}

void GuiManager::Deinit()
{
    UnregisterWidgetsClasses();

    ClearEventsQueue();

    UnregisterAllEventsHandlers();
    SetSelectedWidget(nullptr);
    DetachAllScreens();
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{
    // render screen layers
    for (GuiScreen* currScreen: mScreensList)
    {
        if (currScreen == nullptr)
            continue;

        currScreen->RenderFrame(renderContext);
    }
}

void GuiManager::UpdateFrame()
{
    ProcessEventsQueue();

    // update widgets
    bool doCleanupList = false;

    for (size_t iCurrentScreen = 0, Num = mScreensList.size(); iCurrentScreen < Num; 
        ++iCurrentScreen)
    {
        if (mScreensList[iCurrentScreen])
        {
            mScreensList[iCurrentScreen]->UpdateFrame();
        }

        if (mScreensList[iCurrentScreen] == nullptr)
        {
            doCleanupList = true;
        }
    }

    if (doCleanupList)
    {
        cxx::erase_elements(mScreensList, nullptr);
    }

    ScanHoveredWidget();
}

void GuiManager::ProcessInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mSelectedWidget)
    {
        mSelectedWidget->ProcessEvent(inputEvent);

        // skip hovered widget
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
    }
}

void GuiManager::ProcessInputEvent(MouseMovedInputEvent& inputEvent)
{
    ScanHoveredWidget(); // do extra scan

    if (mSelectedWidget)
    {
        mSelectedWidget->ProcessEvent(inputEvent);

        // skip hovered widget
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
    }
}

void GuiManager::ProcessInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mSelectedWidget)
    {
        mSelectedWidget->ProcessEvent(inputEvent);

        // skip hovered widget
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
    }
}

void GuiManager::ProcessInputEvent(KeyInputEvent& inputEvent)
{
}

void GuiManager::ProcessInputEvent(KeyCharEvent& inputEvent)
{
}

void GuiManager::ScanHoveredWidget()
{
    GuiWidget* newHovered = nullptr;

    if (mSelectedWidget)
    {
        newHovered = mSelectedWidget->PickWidget(gInputsManager.mCursorPosition);
    }

    if (newHovered == nullptr)
    {
        for (auto reverse_iter = mScreensList.rbegin(); reverse_iter != mScreensList.rend(); ++reverse_iter)
        {
            GuiWidget* rootWidget = nullptr;
            GuiScreen* currentScreen = *reverse_iter;
            if (currentScreen)
            {
                rootWidget = currentScreen->mHier.mRootWidget;
            }

            if (rootWidget == nullptr)
                continue;

            newHovered = rootWidget->PickWidget(gInputsManager.mCursorPosition);
            if (newHovered)
                break;
        }
    }

    if (mHoveredWidget == newHovered)
        return;

    if (mHoveredWidget)
    {
        mHoveredWidget->SetHovered(false);
    }

    mHoveredWidget = newHovered;
    if (mHoveredWidget)
    {
        mHoveredWidget->SetHovered(true);
    }
}

bool GuiManager::RegisterWidgetClass(GuiWidgetMetaClass* widgetsClass)
{
    debug_assert(widgetsClass);
    if (widgetsClass == nullptr || widgetsClass->mFactory == nullptr)
        return false;

    GuiWidgetMetaClass*& classRegistered = mWidgetsClasses[widgetsClass->mClassName];
    if (classRegistered)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Widget class '%s' already registered in gui manager", widgetsClass->mClassName.c_str());
        return false;
    }
    classRegistered = widgetsClass;
    return true;
}

void GuiManager::RegisterWidgetsClasses()
{
    RegisterWidgetClass(&GuiWidget::MetaClass);
    RegisterWidgetClass(&GuiPictureBox::MetaClass);
    RegisterWidgetClass(&GuiButton::MetaClass);
    RegisterWidgetClass(&GuiPanel::MetaClass);
    RegisterWidgetClass(&GuiScrollbar::MetaClass);
}

void GuiManager::UnregisterWidgetsClasses()
{
    mWidgetsClasses.clear();
}

GuiWidgetMetaClass* GuiManager::GetWidgetClass(cxx::unique_string className) const
{
    auto iterator_found = mWidgetsClasses.find(className);
    if (iterator_found == mWidgetsClasses.end())
        return nullptr;

    return iterator_found->second;
}

GuiWidget* GuiManager::CreateWidget(cxx::unique_string className) const
{
    if (GuiWidgetMetaClass* widgetClass = GetWidgetClass(className))
    {
        debug_assert(widgetClass->mFactory);
        return widgetClass->mFactory->CreateWidget();
    }

    debug_assert(false);
    return nullptr;
}

void GuiManager::SetSelectedWidget(GuiWidget* mouseListener)
{
    if (mSelectedWidget == mouseListener)
        return;

    mSelectedWidget = mouseListener;
}

void GuiManager::HandleScreenResolutionChanged()
{
    for (GuiScreen* currScreen: mScreensList)
    {
        if (currScreen == nullptr)
            continue;

        currScreen->mHier.FitLayoutToScreen(gGraphicsDevice.mScreenResolution);
    }
}

void GuiManager::BroadcastEvent(const GuiEvent& eventData)
{
    if (mEventHandlers.empty()) // no one's there
        return;

    if (eventData.mEventId.empty())
    {
        debug_assert(false);
        return;
    }

    mEventsQueue.push_back(eventData);
}

void GuiManager::RegisterEventsHandler(GuiEventsHandler* eventsHandler)
{
    debug_assert(eventsHandler);
    if (eventsHandler)
    {
        cxx::push_back_if_unique(mEventHandlers, eventsHandler);
    }
}

void GuiManager::UnregisterEventsHandler(GuiEventsHandler* eventsHandler)
{
    GuiEventsHandler* nullHandler = nullptr;
    if (eventsHandler == nullHandler)
    {
        debug_assert(false);
        return;
    }

    if (IsProcessingEvents())
    {
        std::replace(mEventHandlers.begin(), mEventHandlers.end(), eventsHandler, nullHandler);
    }
    else
    {
        cxx::erase_elements(mEventHandlers, eventsHandler);
    }
}

void GuiManager::UnregisterAllEventsHandlers()
{
    if (IsProcessingEvents())
    {
        std::transform(mEventHandlers.begin(), mEventHandlers.end(), mEventHandlers.begin(), 
            [](GuiEventsHandler* handler)
            {
                return nullptr;
            });
    }
    else
    {
        mEventHandlers.clear();
    }
}

void GuiManager::ProcessEventsQueue()
{
    if (mEventsQueue.empty())
        return;

    if (mEventHandlers.empty()) // no one's there
    {
        ClearEventsQueue();
        return;
    }

    bool hasNullHandlers = false;

    mProcessingEventsQueue.swap(mEventsQueue);
    for (GuiEvent& currentEvent: mProcessingEventsQueue)
    {
        for (size_t icurr = 0, Num = mEventHandlers.size(); icurr < Num; ++icurr)
        {
            if (currentEvent.mEventSender == nullptr) // needs to be checked each iteration - handle may expire
                break;

            GuiEventsHandler* currHandler = mEventHandlers[icurr];
            if (currHandler == nullptr)
            {
                hasNullHandlers = true;
                continue;
            }

            currHandler->ProcessEvent(&currentEvent);
        }
    }
    mProcessingEventsQueue.clear();

    if (hasNullHandlers)
    {
        cxx::erase_elements(mEventHandlers, nullptr);
    }
}

void GuiManager::ClearEventsQueue()
{
    mEventsQueue.clear();

    debug_assert(mProcessingEventsQueue.empty());
    mProcessingEventsQueue.clear();
}

bool GuiManager::IsProcessingEvents() const
{
    return !mProcessingEventsQueue.empty();
}

void GuiManager::AttachScreen(GuiScreen* screen)
{
    debug_assert(screen);
    if (screen == nullptr)
        return;

    debug_assert(screen->IsScreenInitialized());
    debug_assert(!screen->IsScreenActive());

    if (cxx::contains(mScreensList, screen))
    {
        debug_assert(false);
        return;
    }
    mScreensList.push_back(screen);
}

void GuiManager::DetachScreen(GuiScreen* screen)
{
    debug_assert(screen);
    if (screen == nullptr)
        return;

    debug_assert(screen->IsScreenActive());
    for (size_t iCurrentScreen = 0, Num = mScreensList.size(); iCurrentScreen < Num; 
        ++iCurrentScreen)
    {
        if (mScreensList[iCurrentScreen] == screen)
        {
            mScreensList[iCurrentScreen] = nullptr;
        }
    }
}

void GuiManager::DetachAllScreens()
{
    mScreensList.clear();
}
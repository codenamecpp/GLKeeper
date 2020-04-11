#include "pch.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "Console.h"
#include "TimeManager.h"
#include "GuiPictureBox.h"
#include "InputsManager.h"
#include "GuiButton.h"
#include "GuiPanel.h"
#include "GuiSlider.h"

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
    mWidgetsClasses.clear();
    mWidgets.clear();
    mEventHandlers.clear();

    ClearEventsQueue();
}

void GuiManager::AttachWidget(GuiWidget* widget)
{
    cxx::push_back_if_unique(mWidgets, widget);
}

void GuiManager::DetachWidget(GuiWidget* widget)
{
    cxx::erase_elements(mWidgets, widget);
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{
    for (GuiWidget* currWidget: mWidgets)
    {
        currWidget->RenderFrame(renderContext);
    }
}

void GuiManager::UpdateFrame()
{
    ProcessEventsQueue();

    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();
    for (GuiWidget* currWidget: mWidgets)
    {
        currWidget->UpdateFrame(deltaTime);
    }

    UpdateCurrentHoveredWidget();
}

void GuiManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.mButton == eMouseButton_Left)
    {
        if (inputEvent.mPressed)
        {
            HandleMouseLButtonPressed(inputEvent);
        }
        else
        {
            HandleMouseLButtonReleased(inputEvent);
        }
    }
}

void GuiManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCurrentDragHandler) // continue drag
    {
        mCurrentDragHandler->HandleDrag(gInputsManager.mCursorPosition);

        inputEvent.SetConsumed();
        return;
    }

    UpdateCurrentHoveredWidget(); // extra lookup just in case

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
        return;
    }
}

void GuiManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCurrentDragHandler) // continue drag
    {
        inputEvent.SetConsumed();
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
        return;
    }
}

void GuiManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void GuiManager::HandleMouseLButtonPressed(MouseButtonInputEvent& inputEvent)
{
    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
        return;
    }
}

void GuiManager::HandleMouseLButtonReleased(MouseButtonInputEvent& inputEvent)
{
    if (mCurrentDragHandler) // finish drag
    {
        mCurrentDragHandler->HandleDragDrop(gInputsManager.mCursorPosition);
        mCurrentDragHandler = nullptr;

        inputEvent.SetConsumed();
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
        return;
    }
}

void GuiManager::UpdateCurrentHoveredWidget()
{
    GuiWidget* newHovered = nullptr;

    for (auto reverse_iter = mWidgets.rbegin(); reverse_iter != mWidgets.rend(); ++reverse_iter)
    {
        newHovered = (*reverse_iter)->PickWidget(gInputsManager.mCursorPosition);
        if (newHovered)
            break;
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

bool GuiManager::RegisterWidgetClass(GuiWidgetClass* widgetsClass)
{
    debug_assert(widgetsClass);
    if (widgetsClass == nullptr || widgetsClass->mFactory == nullptr)
        return false;

    GuiWidgetClass*& classRegistered = mWidgetsClasses[widgetsClass->mClassName];
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
    RegisterWidgetClass(&gBaseWidgetClass);
    RegisterWidgetClass(&gPictureBoxWidgetClass);
    RegisterWidgetClass(&gButtonWidgetClass);
    RegisterWidgetClass(&gPanelWidgetClass);
    RegisterWidgetClass(&gSliderWidgetClass);
}

GuiWidgetClass* GuiManager::GetWidgetClass(const std::string& className) const
{
    auto iterator_found = mWidgetsClasses.find(className);
    if (iterator_found == mWidgetsClasses.end())
        return nullptr;

    return iterator_found->second;
}

GuiWidget* GuiManager::ConstructWidget(const std::string& className) const
{
    if (GuiWidgetClass* widgetClass = GetWidgetClass(className))
    {
        debug_assert(widgetClass->mFactory);
        return widgetClass->mFactory->ConstructWidget();
    }

    debug_assert(false);
    return nullptr;
}

void GuiManager::SetDragHandler(GuiDragDropHandler* dragHandler, const Point& screenPoint)
{
    if (mCurrentDragHandler == dragHandler)
        return;

    if (mCurrentDragHandler)
    {
        mCurrentDragHandler->HandleDragCancel();
        mCurrentDragHandler = nullptr;
    }

    mCurrentDragHandler = dragHandler;
    if (mCurrentDragHandler)
    {
        mCurrentDragHandler->HandleDragStart(screenPoint);
    }
}

void GuiManager::HandleScreenResolutionChanged()
{
}

void GuiManager::HandleWidgetDestroy(GuiWidget* widget)
{
    if (widget == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (widget == mCurrentDragHandler)
    {
        mCurrentDragHandler = nullptr; // clear silently
    }

    if (widget == mHoveredWidget)
    {
        mHoveredWidget = nullptr;
    }
}

void GuiManager::PostGuiEvent(GuiEvent* eventData)
{
    if (eventData == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (mEventHandlers.empty()) // no one's there
        return;

    mEventsQueue.push_back(*eventData);
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
    debug_assert(eventsHandler);
    if (eventsHandler)
    {
        cxx::erase_elements(mEventHandlers, eventsHandler);
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

    mProcessingEventsQueue.swap(mEventsQueue);
    for (GuiEvent& currentEvent: mProcessingEventsQueue)
    {
        for (GuiEventsHandler* currHandler: mEventHandlers)
        {
            if (currentEvent.mEventSender == nullptr) // needs to be checked each iteration - handle may exire
                break;

            currHandler->ProcessEvent(&currentEvent);
        }
    }
    mProcessingEventsQueue.clear();
}

void GuiManager::ClearEventsQueue()
{
    mEventsQueue.clear();
    mProcessingEventsQueue.clear();
}
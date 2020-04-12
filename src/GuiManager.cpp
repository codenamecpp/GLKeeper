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
    ClearMouseCapture();

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

    ScanHoveredWidget();
}

void GuiManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mMouseCaptureWidget)
    {
        mMouseCaptureWidget->ProcessEvent(inputEvent);

        // skip hovered widget
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
    }
}

void GuiManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    ScanHoveredWidget(); // do extra scan

    if (mMouseCaptureWidget)
    {
        mMouseCaptureWidget->ProcessEvent(inputEvent);

        // skip hovered widget
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
    }
}

void GuiManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mMouseCaptureWidget)
    {
        mMouseCaptureWidget->ProcessEvent(inputEvent);

        // skip hovered widget
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->ProcessEvent(inputEvent);
    }
}

void GuiManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void GuiManager::ScanHoveredWidget()
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

void GuiManager::CaptureMouseInputs(GuiWidget* mouseListener)
{
    if (mMouseCaptureWidget == mouseListener)
        return;

    debug_assert(mMouseCaptureWidget == nullptr);
    mMouseCaptureWidget = mouseListener;
}

void GuiManager::ClearMouseCapture()
{
    mMouseCaptureWidget = nullptr;
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

    if (widget == mMouseCaptureWidget)
    {
        ClearMouseCapture();
    }

    if (widget == mHoveredWidget)
    {
        mHoveredWidget = nullptr;
    }
}

void GuiManager::PostGuiEvent(const GuiEvent& eventData)
{
    if (mEventHandlers.empty()) // no one's there
        return;

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
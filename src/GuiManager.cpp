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
#include "GuiHierarchy.h"
#include "GraphicsDevice.h"

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
    mEventHandlers.clear();

    DetachAllWidgets();
    ClearEventsQueue();
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{
    // render widgets
    for (GuiHierarchy* currHier: mHiersList)
    {
        currHier->RenderFrame(renderContext);
    }
}

void GuiManager::UpdateFrame()
{
    ProcessEventsQueue();

    // update widgets
    for (GuiHierarchy* currHier: mHiersList)
    {
        currHier->UpdateFrame();
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

    if (mMouseCaptureWidget)
    {
        newHovered = mMouseCaptureWidget->PickWidget(gInputsManager.mCursorPosition);
    }

    if (newHovered == nullptr)
    {
        for (auto reverse_iter = mHiersList.rbegin(); reverse_iter != mHiersList.rend(); ++reverse_iter)
        {
            GuiHierarchy* currentHier = *reverse_iter;
            if (currentHier->mRootWidget == nullptr)
                continue;

            newHovered = currentHier->mRootWidget->PickWidget(gInputsManager.mCursorPosition);
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
    for (GuiHierarchy* currHier: mHiersList)
    {
        currHier->FitLayoutToScreen(gGraphicsDevice.mScreenResolution);
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

void GuiManager::AttachWidgets(GuiHierarchy* hierarchy)
{
    if (hierarchy == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::push_back_if_unique(mHiersList, hierarchy);
}

void GuiManager::DetachWidgets(GuiHierarchy* hierarchy)
{
    cxx::erase_elements(mHiersList, hierarchy);
}

void GuiManager::DetachAllWidgets()
{
    mHiersList.clear();
}
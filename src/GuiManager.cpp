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
    ClearMouseCapture();
    DetachAllWidgets();
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

GuiWidget* GuiManager::ConstructWidget(cxx::unique_string className) const
{
    if (GuiWidgetMetaClass* widgetClass = GetWidgetClass(className))
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
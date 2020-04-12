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
#include "GuiLayer.h"

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

    DetachAllGuiLayers();
    ClearEventsQueue();
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{
    // render interactive ui layers
    for (GuiLayer* currentLayer: mLayers)
    {
        currentLayer->RenderFrame(renderContext);
    }
}

void GuiManager::UpdateFrame()
{
    ProcessEventsQueue();

    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();
    // update interactive ui layers
    for (GuiLayer* currentLayer: mLayers)
    {
        currentLayer->UpdateFrame(deltaTime);
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

    if (!mLayers.empty())
    {
        for (auto reverse_iter = mLayers.rbegin(); reverse_iter != mLayers.rend(); ++reverse_iter)
        {
            GuiLayer* currentLayer = *reverse_iter;
            if (currentLayer->mRootWidget == nullptr)
                continue;

            newHovered = currentLayer->mRootWidget->PickWidget(gInputsManager.mCursorPosition);
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

void GuiManager::AttachGuiLayer(GuiLayer* interactiveLayer)
{
    if (interactiveLayer == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::push_back_if_unique(mLayers, interactiveLayer);
}

void GuiManager::DetachGuiLayer(GuiLayer* interactiveLayer)
{
    cxx::erase_elements(mLayers, interactiveLayer);
}

void GuiManager::DetachAllGuiLayers()
{
    mLayers.clear();
}

bool GuiManager::IsGuiLayerAttached(const GuiLayer* interactiveLayer) const
{
    return cxx::contains(mLayers, interactiveLayer);
}
#include "pch.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "Console.h"
#include "TimeManager.h"
#include "GuiPictureBox.h"
#include "InputsManager.h"
#include "GuiButton.h"
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
    LoadScreenRecords();
    return true;
}

void GuiManager::Deinit()
{
    UnregisterWidgetsClasses();
    ClearEventsQueue();
    UnregisterAllEventsHandlers();
    SetMouseCapture(nullptr);
    DetachAllScreens();
    FreeScreenRecords();
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{
    // render screens from back to front
    for (auto curr_iterator = mScreensList.rbegin(); 
        curr_iterator != mScreensList.rend(); ++curr_iterator)
    {
        if (curr_iterator->mInstance)
        {
            curr_iterator->mInstance->RenderFrame(renderContext);
        }
    }
}

void GuiManager::UpdateFrame()
{
    // update screens
    for (const ScreenElement& currElement: mScreensList)
    {
        if (currElement.mInstance)
        {
            currElement.mInstance->UpdateFrame();
        }
    }

    UpdateHoveredWidget();
    ProcessEventsQueue();
}

void GuiManager::ProcessInputEvent(MouseButtonInputEvent& inputEvent)
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

void GuiManager::ProcessInputEvent(MouseMovedInputEvent& inputEvent)
{
    UpdateHoveredWidget(); // do extra scan

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

void GuiManager::ProcessInputEvent(MouseScrollInputEvent& inputEvent)
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

void GuiManager::ProcessInputEvent(KeyInputEvent& inputEvent)
{
}

void GuiManager::ProcessInputEvent(KeyCharEvent& inputEvent)
{
}

void GuiManager::UpdateHoveredWidget()
{
    GuiWidget* newHovered = nullptr;

    if (mHoveredWidget)
    {
        if (!mHoveredWidget->IsVisibleWithParent() ||
            !mHoveredWidget->IsEnabledWithParent())
        {
            mHoveredWidget->NotifyHoverStateChange(false);
            mHoveredWidget.reset();
        }
    }

    // update mouse capture widget
    if (mMouseCaptureWidget)
    {
        if (!mMouseCaptureWidget->IsVisibleWithParent() ||
            !mMouseCaptureWidget->IsEnabledWithParent())
        {
            mMouseCaptureWidget->NotifyMouseCaptureStateChange(false);
            mMouseCaptureWidget.reset();
        }

        newHovered = mMouseCaptureWidget->PickWidget(gInputsManager.mCursorPosition);
    }

    if (newHovered == nullptr)
    {
        for (const ScreenElement& currElement: mScreensList)
        {
            GuiWidget* rootWidget = nullptr;
            GuiScreen* currentScreen = currElement.mInstance;
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
        mHoveredWidget->NotifyHoverStateChange(false);
    }
    mHoveredWidget = newHovered;
    if (mHoveredWidget)
    {
        mHoveredWidget->NotifyHoverStateChange(true);
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

void GuiManager::SetMouseCapture(GuiWidget* widget)
{
    if (mMouseCaptureWidget == widget)
        return;

    if (mMouseCaptureWidget)
    {
        mMouseCaptureWidget->NotifyMouseCaptureStateChange(false);
    }
    mMouseCaptureWidget = widget;
    if (mMouseCaptureWidget)
    {
        mMouseCaptureWidget->NotifyMouseCaptureStateChange(true);
    }
}

void GuiManager::ReleaseMouseCapture(GuiWidget* widget)
{
    debug_assert(widget);
    if (mMouseCaptureWidget == widget && widget)
    {
        mMouseCaptureWidget->NotifyMouseCaptureStateChange(false);
        mMouseCaptureWidget.reset();
    }
}

void GuiManager::HandleScreenResolutionChanged()
{
    for (ScreenElement& currElement: mScreensList)
    {   
        GuiScreen* screen = currElement.mInstance;
        if (screen == nullptr || !screen->IsScreenLoaded())
            continue;

        screen->mHier.FitLayoutToScreen(gGraphicsDevice.mScreenResolution);
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

void GuiManager::LoadScreenRecords()
{
    std::string jsonDocumentContent;
    if (!gFileSystem.ReadTextFile("screens/gui_screens.json", jsonDocumentContent))
    {
        debug_assert(false);
        return;
    }

    cxx::json_document jsonDocument;
    if (!jsonDocument.parse_document(jsonDocumentContent))
    {
        debug_assert(false);
        return;
    }

    for (cxx::json_node_object currScreenNode = jsonDocument.get_root_node().first_child();
        currScreenNode;
        currScreenNode = currScreenNode.next_sibling())
    {
        mScreensList.emplace_back();
        ScreenElement& screenElement = mScreensList.back();
        screenElement.mScreenId = currScreenNode.get_element_name();
        cxx::json_get_attribute(currScreenNode, "content_path", screenElement.mContentPath);
        if (screenElement.mContentPath.empty())
        {
            debug_assert(false);
            gConsole.LogMessage(eLogMessage_Debug, "Screen '%s' content path is missing", screenElement.mScreenId.c_str());
        }
    }
}

void GuiManager::FreeScreenRecords()
{
    mScreensList.clear();
}

void GuiManager::AttachScreen(GuiScreen* screen)
{
    debug_assert(screen);
    if (screen == nullptr)
        return;

    for (ScreenElement& currElement: mScreensList)
    {
        if (currElement.mScreenId == screen->mScreenId)
        {
            currElement.mInstance = screen;
            return;
        }
    }

    debug_assert(false);
    gConsole.LogMessage(eLogMessage_Warning, "Unknown screen id: '%s'", screen->mScreenId.c_str());
}

void GuiManager::DetachScreen(GuiScreen* screen)
{
    debug_assert(screen);
    for (ScreenElement& currElement: mScreensList)
    {
        if (currElement.mInstance == screen)
        {
            currElement.mInstance = nullptr;
            break;
        }
    }
}

void GuiManager::DetachAllScreens()
{
    for (ScreenElement& currElement: mScreensList)
    {
        currElement.mInstance = nullptr;
    }
}

bool GuiManager::GetScreenContentPath(cxx::unique_string screenId, std::string& contentPath)
{
    for (const ScreenElement& currElement: mScreensList)
    {
        if (currElement.mScreenId == screenId)
        {
            contentPath = currElement.mContentPath;
            return true;
        }
    }
    debug_assert(false);
    gConsole.LogMessage(eLogMessage_Warning, "Unknown screen id: '%s'", screenId.c_str());
    return false;
}

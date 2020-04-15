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

    LoadTemplateWidgets();
    return true;
}

void GuiManager::Deinit()
{
    FreeTemplateWidgets();
    UnregisterWidgetsClasses();

    UnregisterAllEventsHandlers();
    ClearMouseCapture();
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

void GuiManager::UnregisterWidgetsClasses()
{
    mWidgetsClasses.clear();
}

bool GuiManager::LoadTemplateWidgets()
{
    std::string jsonDocumentContent;
    if (!gFileSystem.ReadTextFile("layouts/templates.json", jsonDocumentContent))
    {
        debug_assert(false);
        return false;
    }

    cxx::json_document jsonDocument;
    if (!jsonDocument.parse_document(jsonDocumentContent))
    {
        debug_assert(false);
        return false;
    }

    // widget deserializer
    std::function<GuiWidget*(cxx::json_node_object objectNode)> DeserializeWidget = 
        [&DeserializeWidget](cxx::json_node_object objectNode) -> GuiWidget*
    {
        if (!objectNode)
        {
            debug_assert(false);
            return nullptr;
        }

        GuiWidget* widget = nullptr;

        std::string className;
        if (cxx::json_get_attribute(objectNode, "class", className))
        {
            widget = gGuiManager.ConstructWidget(className);
        }

        if (widget == nullptr)
        {
            debug_assert(false);
            return nullptr;
        }

        widget->LoadProperties(objectNode);

        // deserialize childs
        if (cxx::json_node_array childsNode = objectNode["children"])
        {
            for (cxx::json_node_object currNode = childsNode.first_child(); currNode; 
                currNode = currNode.next_sibling())
            {
                GuiWidget* childWidget = DeserializeWidget(currNode);
                if (childWidget == nullptr)
                {
                    debug_assert(false);
                    continue;
                }
                widget->AttachChild(childWidget);
            }
        }
        return widget;
    };

    cxx::json_document_node rootNode = jsonDocument.get_root_node();
    for (cxx::json_document_node currNode = rootNode.first_child(); currNode;
        currNode = currNode.next_sibling())
    {
        std::string templateClassName = currNode.get_element_name();
        if (mTemplateWidgetsClasses.find(templateClassName) != mTemplateWidgetsClasses.end())
        {
            debug_assert(false);

            gConsole.LogMessage(eLogMessage_Warning, "Template widget class already exists '%s'", templateClassName.c_str());
            continue;
        }

        GuiWidget* widget = DeserializeWidget(currNode);
        if (widget == nullptr)
        {
            debug_assert(false);

            gConsole.LogMessage(eLogMessage_Warning, "Cannot load template widget class '%s'", templateClassName.c_str());
            continue;
        }
        widget->mTemplateClassName = templateClassName;
        mTemplateWidgetsClasses[templateClassName] = widget;
    }
    return true;
}

void GuiManager::FreeTemplateWidgets()
{
    for (auto& curr: mTemplateWidgetsClasses)
    {
        SafeDelete(curr.second);
    }

    mTemplateWidgetsClasses.clear();
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

GuiWidget* GuiManager::ConstructTemplateWidget(const std::string& templateClassName) const
{
    auto template_iter = mTemplateWidgetsClasses.find(templateClassName);
    if (template_iter != mTemplateWidgetsClasses.end())
    {
        GuiWidget* templateClone = template_iter->second->CloneDeep();
        return templateClone;
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

void GuiManager::UnregisterAllEventsHandlers()
{
    mEventHandlers.clear();
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
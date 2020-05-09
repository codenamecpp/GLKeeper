#include "pch.h"
#include "GuiHierarchy.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "TimeManager.h"
#include "FileSystem.h"
#include "Console.h"

GuiHierarchy::~GuiHierarchy()
{
    Cleanup();
}

bool GuiHierarchy::LoadFromFile(const std::string& fileName)
{
    std::string jsonDocumentContent;
    if (!gFileSystem.ReadTextFile(fileName, jsonDocumentContent))
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

    return LoadFromJson(jsonDocument);
}

bool GuiHierarchy::LoadFromJson(const cxx::json_document& jsonDocument)
{
    Cleanup();

    // load template widgets
    cxx::json_document_node jsonRootNode = jsonDocument.get_root_node();
    if (cxx::json_node_object templatesNode = jsonRootNode["templates"])
    {
        if (!LoadTemplateWidgets(templatesNode))
        {
            debug_assert(false);

            Cleanup();
            return false;
        }
    }
    // load widgets hierarchy
    if (cxx::json_node_object hierarchyNode = jsonRootNode["hierarchy"])
    {
        if (!LoadHierarchy(hierarchyNode))
        {
            debug_assert(false);

            Cleanup();
            return false;
        }
    }

    if (mRootWidget == nullptr)
    {
        debug_assert(false);
        return false;
    }
    return true;
}

void GuiHierarchy::Cleanup()
{
    FreeTemplateWidgets();

    SafeDelete(mRootWidget);
}

void GuiHierarchy::RenderFrame(GuiRenderer& renderContext)
{
    if (mRootWidget)
    {
        mRootWidget->RenderFrame(renderContext);
    }
}

void GuiHierarchy::UpdateFrame()
{
    if (mRootWidget)
    {
        float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();
        mRootWidget->UpdateFrame(deltaTime);
    }
}

void GuiHierarchy::SetVisible(bool isVisible)
{
    if (mRootWidget)
    {
        mRootWidget->SetVisible(isVisible);
    }
}

void GuiHierarchy::SetEnabled(bool isEnabled)
{
    if (mRootWidget)
    {
        mRootWidget->SetEnabled(isEnabled);
    }
}

void GuiHierarchy::FitLayoutToScreen(const Point& screenDimensions)
{
    if (mRootWidget)
    {
        mRootWidget->ParentSizeChanged(mRootWidget->mSize, screenDimensions);
    }
}

GuiWidget* GuiHierarchy::PickWidget(const Point& screenPosition) const
{
    if (mRootWidget)
    {
        return mRootWidget->PickWidget(screenPosition);
    }
    return nullptr;
}

GuiWidget* GuiHierarchy::GetWidgetByPath(const std::string& widgetPath) const
{
    if (mRootWidget == nullptr)
    {
        debug_assert(false);
        return nullptr;
    }

    if (!cxx::contains(widgetPath, '/'))
    {
        if (mRootWidget->mId == widgetPath)
            return mRootWidget;

        return nullptr;
    }
    
    std::string currentName;
    cxx::string_tokenizer tokenizer(widgetPath);
    // get root name
    if (!tokenizer.get_next(currentName))
    {
        debug_assert(false);
        return nullptr;
    }

    if (mRootWidget->mId != currentName)
        return nullptr;

    GuiWidget* currentWidget = mRootWidget;
    for (;;)
    {
        if (!tokenizer.get_next(currentName))
            break;

        debug_assert(!currentName.empty());
        if (GuiWidget* child = currentWidget->GetChild(currentName))
        {
            currentWidget = child;
            continue;
        }
        // not found
        currentWidget = nullptr;
        break;
    }

    return currentWidget;
}

GuiWidget* GuiHierarchy::SearchForWidget(const cxx::unique_string& name) const
{
    if (mRootWidget == nullptr)
    {
        debug_assert(false);
        return nullptr;
    }

    if (mRootWidget->mId == name)
        return mRootWidget;

    return mRootWidget->SearchForChild(name);
}

GuiWidget* GuiHierarchy::SearchForWidget(const std::string& name) const
{
    if (mRootWidget == nullptr)
    {
        debug_assert(false);
        return nullptr;
    }

    if (mRootWidget->mId == name)
        return mRootWidget;

    return mRootWidget->SearchForChild(name);
}

GuiWidget* GuiHierarchy::CreateTemplateWidget(cxx::unique_string className) const
{
    auto template_iter = mTemplateWidgetsClasses.find(className);
    if (template_iter != mTemplateWidgetsClasses.end())
    {
        GuiWidget* templateClone = template_iter->second->CloneDeep();
        return templateClone;
    }
    debug_assert(false);
    return nullptr;
}

void GuiHierarchy::FreeTemplateWidgets()
{
    for (auto& curr: mTemplateWidgetsClasses)
    {
        SafeDelete(curr.second);
    }

    mTemplateWidgetsClasses.clear();
}

GuiWidget* GuiHierarchy::DeserializeWidgetWithChildren(cxx::json_node_object objectNode)
{
    bool widgetIsTemplate = false;
    if (!objectNode)
    {
        debug_assert(false);
        return nullptr;
    }

    GuiWidget* widget = nullptr;

    cxx::unique_string className;
    if (cxx::json_get_attribute(objectNode, "class", className))
    {
        widget = gGuiManager.CreateWidget(className);
    }
    else if (cxx::json_get_attribute(objectNode, "template_class", className))
    {
        widget = CreateTemplateWidget(className);
        widgetIsTemplate = true;
    }

    if (widget == nullptr)
    {
        debug_assert(false);
        return nullptr;
    }

    widget->LoadProperties(objectNode);

    if (widgetIsTemplate) // just load properties for existing child widgets
    {
        LoadChildrenWidgetProperties(widget, objectNode);
        return widget;
    }

    // full deserialize childs
    if (cxx::json_node_array childsNode = objectNode["children"])
    {
        
        for (cxx::json_node_object currNode = childsNode.first_child(); currNode; 
            currNode = currNode.next_sibling())
        {
            GuiWidget* childWidget = DeserializeWidgetWithChildren(currNode);
            if (childWidget == nullptr)
            {
                debug_assert(false);
                continue;
            }
            widget->AttachChild(childWidget);
        }
    }
    return widget;
}

void GuiHierarchy::LoadChildrenWidgetProperties(GuiWidget* parentWidget, cxx::json_node_object objectNode)
{
    if (parentWidget == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::json_node_array childsNode = objectNode["children"];
    for (cxx::json_node_object currNode = childsNode.first_child(); currNode; 
        currNode = currNode.next_sibling())
    {
        std::string child_id;
        if (!cxx::json_get_attribute(currNode, "id", child_id))
        {
            debug_assert(false);
            return;
        }
        GuiWidget* child_widget = parentWidget->GetChild(child_id);
        if (child_widget == nullptr)
        {
            debug_assert(false);
            return;
        }
        child_widget->LoadProperties(currNode);
        LoadChildrenWidgetProperties(child_widget, currNode);
    }
}

GuiWidget* GuiHierarchy::DeserializeTemplateWidget(cxx::json_node_object objectNode)
{
    if (!objectNode)
    {
        debug_assert(false);
        return nullptr;
    }

    GuiWidget* widget = nullptr;

    cxx::unique_string className;
    if (cxx::json_get_attribute(objectNode, "class", className))
    {
        widget = gGuiManager.CreateWidget(className);
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
            GuiWidget* childWidget = DeserializeTemplateWidget(currNode);
            if (childWidget == nullptr)
            {
                debug_assert(false);
                continue;
            }
            widget->AttachChild(childWidget);
        }
    }
    return widget;
}

bool GuiHierarchy::LoadTemplateWidgets(cxx::json_node_object rootNode)
{
    for (cxx::json_document_node currNode = rootNode.first_child(); currNode;
        currNode = currNode.next_sibling())
    {
        cxx::unique_string templateClassName = cxx::unique_string(currNode.get_element_name());
        if (mTemplateWidgetsClasses.find(templateClassName) != mTemplateWidgetsClasses.end())
        {
            debug_assert(false);

            gConsole.LogMessage(eLogMessage_Warning, "Template widget class already exists '%s'", templateClassName.c_str());
            continue;
        }

        GuiWidget* widget = DeserializeTemplateWidget(currNode);
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

bool GuiHierarchy::LoadHierarchy(cxx::json_node_object objectNode)
{
    GuiWidget* rootWidget = DeserializeWidgetWithChildren(objectNode);

    if (rootWidget == nullptr)
    {
        debug_assert(false);
        return false;
    }
    debug_assert(mRootWidget == nullptr);
    mRootWidget = rootWidget;
    return true;
}
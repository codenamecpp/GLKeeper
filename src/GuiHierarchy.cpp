#include "pch.h"
#include "GuiHierarchy.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "TimeManager.h"
#include "FileSystem.h"

GuiHierarchy::~GuiHierarchy()
{
    Clear();
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

    Clear();

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

    mRootWidget = DeserializeWidget(jsonDocument.get_root_node());
    if (mRootWidget == nullptr)
    {
        debug_assert(false);
        return false;
    }
    return true;
}

void GuiHierarchy::Clear()
{
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
        mRootWidget->ParentSizeChanged(mRootWidget->GetSize(), screenDimensions);
    }
}

GuiWidget* GuiHierarchy::GetWidgetByPath(const std::string& widgetPath) const
{
    if (mRootWidget == nullptr)
        return nullptr;

    if (!cxx::contains(widgetPath, '/'))
    {
        return mRootWidget->GetChild(widgetPath);
    }
    
    GuiWidget* currentWidget = mRootWidget;

    std::string currentName;
    cxx::string_tokenizer tokenizer(widgetPath);
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

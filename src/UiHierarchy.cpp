#include "stdafx.h"
#include "UiHierarchy.h"
#include "UiWidgetManager.h"

UiHierarchy::~UiHierarchy()
{
    Cleanup();
}

bool UiHierarchy::LoadFrom(const std::string& fileName)
{
    Cleanup();

    std::string jsonPath;
    if (!gFiles.PathToFile(fileName, jsonPath))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot locate json document '%s'", fileName.c_str());
        return false;
    }

    JsonDocument jsonDocument;
    if (!FSLoadJSON(jsonPath, jsonDocument))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot parse json document '%s'", fileName.c_str());
        return false;
    }

    // widget templates
    JsonElement templatesNode = jsonDocument.GetRootElement().FindElement("templates");
    if (templatesNode)
    {
        DeserializeTemplateWidgets(templatesNode);
    }

    mRootWidget = DeserializeWidget(jsonDocument.GetRootElement().FindElement("widget"));
    cxx_assert(mRootWidget);

    if (mRootWidget == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load widgets hierarchy from json document '%s'", fileName.c_str());
    }

    return IsLoaded();
}

void UiHierarchy::Cleanup()
{
    if (mRootWidget)
    {
        mRootWidget->DeleteWidget();
        mRootWidget = nullptr;
    }

    for (UiWidget* roller: mTemplates)
    {
        roller->DeleteWidget();
    }
    mTemplates.clear();
}

UiWidget* UiHierarchy::PickWidget(const Point2D& screenPosition) const
{
    UiWidget* resultWidget = nullptr;
    if (mRootWidget) 
    {
        resultWidget = mRootWidget->PickWidget(screenPosition);
    }
    return resultWidget;
}

UiWidget* UiHierarchy::GetWidgetByPath(std::string_view widgetPath) const
{
    return GetWidgetByPath(mRootWidget, widgetPath); 
}

UiWidget* UiHierarchy::GetWidgetByPath(UiWidget* rootWidget, std::string_view widgetPath) const
{
    if (rootWidget == nullptr) 
        return nullptr;

    UiWidget* resultWidget = rootWidget;
    for (std::string::size_type istart = 0, ilength = widgetPath.length(); resultWidget;)
    {
        std::string::size_type iseparator = widgetPath.find('.', istart);
        if (iseparator == std::string::npos)
            iseparator = ilength;

        std::string_view childName = widgetPath.substr(istart, iseparator - istart);
        resultWidget = resultWidget->GetChild(childName);
        if (resultWidget == nullptr) 
            return nullptr;

        istart = iseparator + 1;

        if (iseparator == ilength) 
            return resultWidget;
    }

    return nullptr;
}

UiWidget* UiHierarchy::FindWidgetWithName(std::string_view name) const
{
    if (mRootWidget == nullptr) 
        return nullptr;

    if (mRootWidget->GetName() == name)
        return mRootWidget;

    return mRootWidget->FindChildWithName(name);
}

UiWidget* UiHierarchy::DeserializeWidget(const JsonElement& jsonElement) const
{
    if (!jsonElement.IsObject())
    {
        gConsole.LogMessage(eLogLevel_Warning, "Widget deserialization error: object expected");
        return nullptr;
    }

    UiWidget* widget = nullptr;

    // handle template widget
    std::string templateId;
    if (JsonQuery(jsonElement, "template", templateId))
    {
        UiWidget* widgetTemplate = GetWidgetTemplate(templateId);
        cxx_assert(widgetTemplate);
        if (widgetTemplate == nullptr)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Widget deserialization error, unknown widget template id '%s'", templateId.c_str());
            return nullptr;
        }

        widget = widgetTemplate->CloneWidget();
        cxx_assert(widget);
    }
    else
    {
        std::string widgetClassName;
        if (!JsonQuery(jsonElement, "class", widgetClassName))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Widget deserialization error, widget class missing");
            return nullptr;
        }

        widget = gWidgetManager.ConstructWidget(widgetClassName);
        cxx_assert(widget);
        if (widget == nullptr)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Widget deserialization error, unknown widget class '%s'", widgetClassName.c_str());
            return nullptr;
        }
    }

    widget->Deserialize(jsonElement);

    // loading children widgets
    const JsonElement children = jsonElement.FindElement("children");
    if (children)
    {
        const int ChildrenCount = children.GetArrayElementsCount();
        for (int iChildElement = 0; iChildElement < ChildrenCount; ++iChildElement)
        {
            const JsonElement jsonChild = children.GetArrayElement(iChildElement);
            if (UiWidget* childWidget = DeserializeWidget(jsonChild))
            {
                widget->AttachChild(childWidget);
            }         
        }
    }
    return widget;
}

UiWidget* UiHierarchy::GetWidgetTemplate(std::string_view name) const
{
    for (UiWidget* roller: mTemplates)
    {
        if (roller->GetName() == name)
            return roller;
    }
    return nullptr;
}

void UiHierarchy::DeserializeTemplateWidgets(const JsonElement& templatesRoot)
{
    cxx_assert(templatesRoot.IsArray());

    if (!templatesRoot.IsArray())
        return;

    for (JsonElement templatesRoller = templatesRoot.FirstChild(); 
        templatesRoller;
        templatesRoller = templatesRoller.NextSibling())
    {
        cxx_assert(templatesRoller.IsObject());

        UiWidget* templateWidget = DeserializeWidget(templatesRoller);
        cxx_assert(templateWidget);
        if (templateWidget == nullptr)
            continue;

        const std::string& templateId = templateWidget->GetName();

        // widget must have a unique name
        if (templateId.empty() || GetWidgetTemplate(templateId))
        {
            cxx_assert(false);
            templateWidget->DeleteWidget();
        }
        else // register new template widget
        {
            mTemplates.push_back(templateWidget);
        }
    }
}

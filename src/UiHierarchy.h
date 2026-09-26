#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"
#include "UiWidget.h"

//////////////////////////////////////////////////////////////////////////

class UiHierarchy: public cxx::noncopyable
{
public:
    UiHierarchy() = default;
    ~UiHierarchy();

    // load widgets hierarchy from json
    bool LoadFrom(const std::string& fileName);
    void Cleanup();

    // whether hierarchy is loaded
    inline bool IsLoaded() const { return mRootWidget != nullptr; }

    // pick visible and interactive widget at specified screen coordinate
    UiWidget* PickWidget(const Point2D& screenPosition) const;
    // find widget by specific location within hierarchy
    UiWidget* GetWidgetByPath(std::string_view widgetPath) const;
    UiWidget* GetWidgetByPath(UiWidget* rootWidget, std::string_view widgetPath) const;
    // get first widget withing hierarchy with specific name
    UiWidget* FindWidgetWithName(std::string_view name) const;

    UiWidget* GetWidgetTemplate(std::string_view name) const;

    // accessing root widget of hierarchy
    inline UiWidget* GetRootWidget() const { return mRootWidget; }

private:
    UiWidget* DeserializeWidget(const JsonElement& jsonElement) const;

    void DeserializeTemplateWidgets(const JsonElement& templatesRoot);
    
private:
    UiWidget* mRootWidget = nullptr;
    std::vector<UiWidget*> mTemplates;
};

//////////////////////////////////////////////////////////////////////////
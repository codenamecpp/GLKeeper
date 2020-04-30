#pragma once

#include "GuiDefs.h"

class GuiHierarchy: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mRootWidget = nullptr;

public:
    ~GuiHierarchy();

    // load widgets hierarchy from json document
    // @param fileName: File name
    bool LoadFromFile(const std::string& fileName);
    bool LoadFromJson(const cxx::json_document& jsonDocument);
    void Cleanup();

    // render hierarchy
    void RenderFrame(GuiRenderer& renderContext);

    // process hierarchy
    void UpdateFrame();

    // show or hire, enable or disable hierarchy
    void SetVisible(bool isVisible);
    void SetEnabled(bool isEnabled);

    // handle screen resolution changed during runtime
    void FitLayoutToScreen(const Point& screenDimensions);

    // find widget by specific location within hierarchy
    // @param widgetPath: Path, includes root
    GuiWidget* GetWidgetByPath(const std::string& widgetPath) const;

    // get first widget withing hierarchy with specific name
    GuiWidget* SearchForWidget(const cxx::unique_string& name) const;
    GuiWidget* SearchForWidget(const std::string& name) const;

private:
    // construct new template widget of specified class
    // @param className: Template widget class name
    GuiWidget* CreateTemplateWidget(cxx::unique_string className) const;

    // destroy template widgets
    void FreeTemplateWidgets();

    GuiWidget* DeserializeWidgetWithChildren(cxx::json_node_object objectNode);
    GuiWidget* DeserializeTemplateWidget(cxx::json_node_object objectNode);

    bool LoadTemplateWidgets(cxx::json_node_object objectNode);
    bool LoadHierarchy(cxx::json_node_object objectNode);

private:
    using GuiTemplateWidgetsMap = std::map<cxx::unique_string, GuiWidget*>;
    GuiTemplateWidgetsMap mTemplateWidgetsClasses;
};
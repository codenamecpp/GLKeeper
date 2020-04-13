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
    void Clear();

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
    // @param widgetPath: Path, don't include root
    GuiWidget* GetWidgetByPath(const std::string& widgetPath) const;
};
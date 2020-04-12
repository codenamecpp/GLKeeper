#pragma once

#include "GuiDefs.h"

class GuiHierarchy: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mRootWidget = nullptr;

public:
    ~GuiHierarchy();

    // free widgets
    void Clear();

    // render hierarchy
    void RenderFrame(GuiRenderer& renderContext);

    // process hierarchy
    void UpdateFrame();

    // show or hire, enable or disable hierarchy
    void SetVisible(bool isVisible);
    void SetEnabled(bool isEnabled);
};
#pragma once

#include "GuiDefs.h"

class GuiHierarchy: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mRootWidget = nullptr;

public:
    ~GuiHierarchy();

    // render hierarchy
    void RenderFrame(GuiRenderer& renderContext);

    // process hierarchy
    void UpdateFrame();

};
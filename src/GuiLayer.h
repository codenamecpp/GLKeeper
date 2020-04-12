#pragma once

#include "GuiDefs.h"

// gui interactive layer class
class GuiLayer: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mRootWidget = nullptr;

public:
    virtual ~GuiLayer();

    // render widgets
    void RenderFrame(GuiRenderer& renderContext);

    // process widgets
    void UpdateFrame(float deltaTime);

protected:
    // overridables
    virtual void HandleRenderFrame(GuiRenderer& renderContext) {}
    virtual void HandleUpdateFrame() {};

};
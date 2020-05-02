#pragma once

#include "GuiDefs.h"
#include "GuiHierarchy.h"

// interactive screen layer
class GuiScreenLayer: public cxx::noncopyable
{
    friend class GuiManager;

public:
    GuiScreenLayer();
    virtual ~GuiScreenLayer();

    // render
    void RenderFrame(GuiRenderer& renderContext);

    // process single frame logic
    void UpdateFrame();

protected:
    // overridables
    virtual void HandlePreRenderHier(GuiRenderer& renderContext) {}
    virtual void HandlePostRenderHier(GuiRenderer& renderContext) {}
    virtual void HandlePreUpdateHier() {}
    virtual void HandlePostUpdateHier() {}

protected:
    GuiHierarchy mHier;
};
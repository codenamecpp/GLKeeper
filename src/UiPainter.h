#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"

//////////////////////////////////////////////////////////////////////////

class UiPainter: public cxx::noncopyable
{
public:
    virtual ~UiPainter() {}

    //////////////////////////////////////////////////////////////////////////
    // overridables
    //////////////////////////////////////////////////////////////////////////

    // full paint override
    // returns true if handled (suppresses default widget rendering)

    virtual bool CustomDraw(const UiWidget& widget, UiRenderContext& uiRenderContext)
    {
        return false;
    }

    // alternative pipline

    // background render pass
    // executed BEFORE the widget's default RenderSelf()
    // only called if CustomDraw() returns false

    virtual void CustomDrawBackground(const UiWidget& widget, UiRenderContext& uiRenderContext)
    {
    }

    // overlay render pass
    // executed AFTER the widget's default RenderSelf()
    // only called if CustomDraw() returns false

    virtual void CustomDrawOverlay(const UiWidget& widget, UiRenderContext& uiRenderContext)
    {
    }
};

//////////////////////////////////////////////////////////////////////////
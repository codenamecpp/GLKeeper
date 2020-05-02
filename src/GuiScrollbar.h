#pragma once

#include "GuiWidget.h"
#include "GuiEvent.h"

// scrollbar widget
class GuiScrollbar: public GuiWidget
    , public GuiEventsHandler // handle child events
{
public:

    static GuiWidgetMetaClass MetaClass; // scrollbar widget class

public:
    GuiScrollbar();

    // set slider params
    void SetMin(int minValue);
    void SetMax(int maxValue);
    void SetPageSize(int pageSize);

    // get slider params
    inline int GetMin() const { return mMin; }
    inline int GetMax() const { return mMax; }
    inline int GetPageSize() const { return mPageSize; }

protected:
    GuiScrollbar(GuiWidgetMetaClass* widgetClass);
    GuiScrollbar(GuiScrollbar* copyWidget);

    void SetupControlWidget(GuiWidget* controlWidget);

    // override GuiWidget
    void HandleUpdate(float deltaTime) override;
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleChildAttached(GuiWidget* childWidget) override;
    void HandleChildDetached(GuiWidget* childWidget) override;
    GuiScrollbar* CreateClone() override;

    // override GuiEventsHandler
    void HandleMouseDown(GuiWidget* sender, eMouseButton mbutton) override;
    void HandleMouseUp(GuiWidget* sender, eMouseButton mbutton) override;

protected:
    int mSpacing = 2;
    int mMin = 0;
    int mMax = 100;
    int mPageSize = 10;
    // control widgets
    GuiWidget* mSliderWidget = nullptr;
    GuiWidget* mDecPosWidget = nullptr;
    GuiWidget* mIncPosWidget = nullptr;
};

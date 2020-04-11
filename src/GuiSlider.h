#pragma once

#include "GuiWidget.h"

// slider widget
class GuiSlider: public GuiWidget
{
public:
    GuiSlider();

    // set slider params
    void SetMin(int minValue);
    void SetMax(int maxValue);
    void SetPageSize(int pageSize);

    // get slider params
    inline int GetMin() const { return mMin; }
    inline int GetMax() const { return mMax; }
    inline int GetPageSize() const { return mPageSize; }

protected:
    GuiSlider(GuiWidgetClass* widgetClass);
    GuiSlider(GuiSlider* copyWidget);

    void SetupSliderWidget();

    // override GuiWidget
    void HandleRender(GuiRenderer& renderContext) override;
    bool HasAttribute(eGuiWidgetAttribute attribute) const override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleChildAttached(GuiWidget* childWidget) override;
    void HandleChildDetached(GuiWidget* childWidget) override;
    GuiSlider* ConstructClone() override;

    // override dragndrop
    void HandleDragStart(const Point& screenPoint) override;
    void HandleDragCancel() override;
    void HandleDragDrop(const Point& screenPoint) override;
    void HandleDrag(const Point& screenPoint) override;

protected:
    GuiWidget* mSliderWidget = nullptr;
    int mMin = 0;
    int mMax = 100;
    int mPageSize = 10;
};

// slider widget class
extern GuiWidgetClass gSliderWidgetClass;

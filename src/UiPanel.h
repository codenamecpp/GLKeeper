#pragma once

#include "UiWidget.h"

//////////////////////////////////////////////////////////////////////////
// UiPanel Widget
//////////////////////////////////////////////////////////////////////////

class UiPanel: public UiWidget
{
public:
    UiPanel();

    // Set border and fill colors for panel
    void SetBorderColor(Color32 borderColor);
    void SetFillColor(Color32 fillColor);

    // Load widget properties from json
    void Deserialize(const JsonElement& jsonElement) override;

protected:
    UiPanel(const std::string& widgetClassName);
    UiPanel(const UiPanel& sourceWidget); // clone props
    ~UiPanel();

    UiPanel* CloneSelf() const override;

    // override UiWidget
    void RenderSelf(UiRenderContext& uiRenderContext) override;

protected:
    Color32 mBorderColor;
    Color32 mFillColor;
    int mBorderWidth;
};
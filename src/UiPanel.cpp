#include "stdafx.h"
#include "UiPanel.h"
#include "UiRenderContext.h"

UiPanel::UiPanel() : UiPanel("panel")
{}

UiPanel::UiPanel(const std::string& widgetClassName)
    : UiWidget(widgetClassName)
    , mBorderColor(COLOR_BLACK)
    , mFillColor(COLOR_BLACK)
    , mBorderWidth(1)
{
}

UiPanel::UiPanel(const UiPanel& sourceWidget)
    : UiWidget(sourceWidget)
    , mBorderColor(sourceWidget.mBorderColor)
    , mFillColor(sourceWidget.mFillColor)
    , mBorderWidth(sourceWidget.mBorderWidth)
{
}

UiPanel::~UiPanel()
{
}

void UiPanel::SetBorderColor(Color32 borderColor)
{
    mBorderColor = borderColor;
}

void UiPanel::SetFillColor(Color32 fillColor)
{
    mFillColor = fillColor;
}

void UiPanel::Deserialize(const JsonElement& jsonElement)
{
    UiWidget::Deserialize(jsonElement);

    if (JsonElement jsonNode = jsonElement.FindElement("border_color"))
    {
        Color32 color = COLOR_BLACK;
        if (JsonReadValue(jsonNode, color))
        {
            SetBorderColor(color);
        }
    }

    if (JsonElement jsonNode = jsonElement.FindElement("fill_color"))
    {
        Color32 color = COLOR_BLACK;
        if (JsonReadValue(jsonNode, color))
        {
            SetFillColor(color);
        }
    }

    JsonQuery(jsonElement, "border_width", mBorderWidth);
}

UiPanel* UiPanel::CloneSelf() const
{
    UiPanel* clone = new UiPanel(*this);
    return clone;
}

void UiPanel::RenderSelf(UiRenderContext& uiRenderContext)
{
    Rect2D rcDestination = GetLocalBounds();
    
    if (mBorderWidth > 0 && mBorderColor.mA > 0)
    {
        cxx_assert(mBorderWidth == 1); // TODO: implement
        if (mBorderWidth == 1)
        {
            uiRenderContext.DrawRect(rcDestination, mBorderColor);
        }
        rcDestination.x += mBorderWidth;
        rcDestination.y += mBorderWidth;
        rcDestination.w -= mBorderWidth * 2;
        rcDestination.h -= mBorderWidth * 2;
    }
    if (mFillColor.mA > 0)
    {
        uiRenderContext.FillRect(rcDestination, mFillColor);
    }
}
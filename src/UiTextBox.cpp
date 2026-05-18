#include "stdafx.h"
#include "UiTextBox.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "UiManager.h"
#include "FontManager.h"
#include "TextManager.h"

UiTextBox::UiTextBox() : UiTextBox("textbox")
{
    mInteractive = false;
}

UiTextBox::UiTextBox(const std::string& widgetClassName)
    : UiWidget(widgetClassName)
    , mHorzAlignment(eTextHorzAlignment_Left)
    , mTextColorDefault(COLOR_WHITE)
    , mTextColorHovered(COLOR_WHITE)
    , mTextColor(COLOR_WHITE)
{
}

UiTextBox::UiTextBox(const UiTextBox& sourceWidget)
    : UiWidget(sourceWidget)
    , mHorzAlignment(sourceWidget.mHorzAlignment)
    , mVertAlignment(sourceWidget.mVertAlignment)
    , mTextColorDefault(sourceWidget.mTextColorDefault)
    , mTextColorHovered(sourceWidget.mTextColorHovered)
    , mTextColor(sourceWidget.mTextColor)
    , mTextContent(sourceWidget.mTextContent)
    , mTextFont(sourceWidget.mTextFont)
    , mTextBatchDirty(true) // force
    , mStringId(sourceWidget.mStringId)
{
}

UiTextBox::~UiTextBox()
{
}

UiTextBox* UiTextBox::CloneSelf() const
{
    UiTextBox* clone = new UiTextBox(*this);
    return clone;
}

void UiTextBox::Refresh()
{
    if (mTextBatchDirty)
    {
        mTextBatchDirty = false;
        RecomptuteCache();
    }
}

void UiTextBox::SetText(const std::wstring& text)
{
    if (mTextContent == text) return;

    mTextContent = text;
    mStringId = 0; // reset text table text
    InvalidateCache();
}

void UiTextBox::SetStringId(int stringId)
{
    if (mStringId == stringId) return;

    mStringId = stringId;
    InvalidateCache();
}

void UiTextBox::SetTextFont(const std::string& textFontName)
{
    Font* uiFont = nullptr;
    
    if (!textFontName.empty())
    {
        uiFont = gFontManager.GetFont(textFontName);
        cxx_assert(uiFont);
    }

    if (uiFont != mTextFont)
    {
        mTextFont = uiFont;
        InvalidateCache();
    }
}

void UiTextBox::SetTextColor(Color32 textColor)
{
    if (mTextColorDefault == textColor)
        return;

    mTextColorDefault = textColor;
    RefreshColor();
}

void UiTextBox::SetTextAlignment(eTextHorzAlignment alignmentMode)
{
    if (mHorzAlignment == alignmentMode)
        return;

    mHorzAlignment = alignmentMode;
    InvalidateCache();
}   

void UiTextBox::SetTextAlignment(eTextVertAlignment alignmentMode)
{
    if (mVertAlignment == alignmentMode)
        return;

    mVertAlignment = alignmentMode;
    InvalidateCache();
}

void UiTextBox::Deserialize(const JsonElement& jsonElement)
{
    UiWidget::Deserialize(jsonElement);

    // font
    if (JsonElement pathNode = jsonElement.FindElement("font"))
    {
        std::string fontName = pathNode.GetValueString();
        if (fontName.length())
        {
            SetTextFont(fontName);
            cxx_assert(mTextFont);
        }
    }

    // horz align mode
    if (JsonElement alignmentNode = jsonElement.FindElement("horz_align"))
    {
        const std::string alignment = alignmentNode.GetValueString();
        bool isSuccess = cxx::parse_enum(alignment.c_str(), mHorzAlignment);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Unknown text alignment mode '%s'", alignment.c_str());
        }
        cxx_assert(isSuccess);
    }

    if (JsonElement textNode = jsonElement.FindElement("text"))
    {
        std::string_view textValue = textNode.GetValueString();
        if (!cxx::string_to_wide_string(textValue, mTextContent))
        {
            cxx_assert(false);
        }
    }

    JsonQuery(jsonElement, "color", mTextColorDefault);
    JsonQuery(jsonElement, "color_hovered", mTextColorHovered);
    JsonQuery(jsonElement, "string_id", mStringId);

    RefreshColor();
    InvalidateCache();
}

void UiTextBox::RenderSelf(UiRenderContext& uiRenderContext)
{
    if (mTextFont == nullptr)
        return;

    Refresh();
    //uiRenderContext.DrawRect(GetLocalBounds(), COLOR_PINK);

    uiRenderContext.DrawTextQuads(mTextFont, mTextBatch);
}

void UiTextBox::HandleSizeChanged(const Point2D& prevSize)
{
    InvalidateCache();
}

void UiTextBox::HandleMouseEnter()
{
    RefreshColor();
}

void UiTextBox::HandleMouseLeave()
{
    RefreshColor();
}

void UiTextBox::HandleEnableStateChanged()
{
    RefreshColor();
}

void UiTextBox::HandleVisibilityChanged()
{
    RefreshColor();
}


void UiTextBox::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.IsButtonPressed(MBUTTON_LEFT))
    {
        // notify
        const UiEvent_OnPress eventDesc (MBUTTON_LEFT);
        mEventListeners.IterateListeners([this, &eventDesc](UiEventListener* listener)
            {
                listener->HandleUiEvent(this, &eventDesc);
            });
        inputEvent.mConsumed = true;
    }
}

void UiTextBox::RecomptuteCache()
{
    mTextBatch.clear();

    if (mTextFont == nullptr) return;

    if (mStringId > 0)
    {
        mTextContent = gTexts.GetString(TextTableId_Main, mStringId);
    }

    if (mTextContent.empty()) return;

    Rect2D localBounds = GetLocalBounds();
    if ((localBounds.w > 0) || (localBounds.h > 0))
    {
        mTextFont->BuildTextMesh(mTextContent, localBounds, mHorzAlignment, mVertAlignment, mTextColor, mTextBatch);
    }
    else
    {
        mTextFont->BuildTextMesh(mTextContent, {}, mTextColor, mTextBatch);
    }
}

void UiTextBox::InvalidateCache()
{
    mTextBatchDirty = true;
}

void UiTextBox::RefreshColor()
{
    Color32 currentColor = IsHovered() ? mTextColorHovered : mTextColorDefault;
    if (mTextColor == currentColor) return;

    mTextColor = currentColor;

    // update text batch
    for (Quad2D& textQuad: mTextBatch)
    {
        for (Vertex2D& vertex: textQuad.mPoints)
        {
            vertex.mColor = mTextColor;
        }
    }
}

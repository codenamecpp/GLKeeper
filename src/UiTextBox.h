#pragma once

#include "UiWidget.h"
#include "Font.h"

//////////////////////////////////////////////////////////////////////////
// Ui Text Box
//////////////////////////////////////////////////////////////////////////

class UiTextBox: public UiWidget
{
public:
    UiTextBox();

    // Setup textbox content and primary text color, text can contain formatting tags
    void SetTextFont(const std::string& textFontName);
    void SetText(const std::wstring& text);
    void SetStringId(int stringId);

    // Force rebuild text mesh
    // Normally vertices is being regenerated on widget render
    void Refresh();

    // Set specific alignment mode for text content
    void SetTextAlignment(eTextHorzAlignment alignmentMode);
    void SetTextAlignment(eTextVertAlignment alignmentMode);

    // Set specific primary color for text content
    void SetTextColor(Color32 textColor);

    // Load widget properties from json
    void Deserialize(const JsonElement& jsonElement) override;

protected:
    UiTextBox(const std::string& widgetClassName);
    UiTextBox(const UiTextBox& sourceWidget); // clone props
    ~UiTextBox();

    UiTextBox* CloneSelf() const override;

    void RecomptuteCache();
    void InvalidateCache();

    void RefreshColor();

    // override UiWidget
    void RenderSelf(UiRenderContext& uiRenderContext) override;
    void HandleSizeChanged(const Point2D& prevSize) override;
    void HandleMouseEnter() override;
    void HandleMouseLeave() override;
    void HandleEnableStateChanged() override;
    void HandleVisibilityChanged() override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;

protected:
    Font* mTextFont = nullptr;

    int mStringId = 0; // from text table

    std::wstring mTextContent;
    std::vector<Quad2D> mTextBatch;

    Color32 mTextColorHovered;
    Color32 mTextColorDefault;
    Color32 mTextColor; // one of default or hovered
    eTextHorzAlignment mHorzAlignment = eTextHorzAlignment_Left;
    eTextVertAlignment mVertAlignment = eTextVertAlignment_Top;

    bool mTextBatchDirty = true;
};

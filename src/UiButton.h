#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiWidget.h"
#include "Texture.h"

//////////////////////////////////////////////////////////////////////////
// Button widget
//////////////////////////////////////////////////////////////////////////

class UiButton: public UiWidget
{
public:
    UiButton();

    // Set texture for specified button state
    void SetButtonStatePicture(eUiButtonState uiButtonState, const std::string& pictureName);

    // Set button pressed state
    virtual void SetPressed(bool isPressed);
    inline bool IsPressed() const { return mPressed; }

    // Click the button
    virtual void Click(int mouseButton = MBUTTON_LEFT);

    // override UiWidget
    void Deserialize(const JsonElement& jsonElement) override;

protected:
    UiButton(const std::string& widgetClassName);
    UiButton(const UiButton& sourceWidget); // clone props
    ~UiButton();

    UiButton* CloneSelf() const override;

    // Select active state for button
    virtual void ButtonStateChanged();

    // override UiWidget
    void RenderSelf(UiRenderContext& uiRenderContext) override;
    void HandleEnabledChanged() override;
    void HandleVisibleChanged() override;
    void HandleMouseEnter() override;
    void HandleMouseLeave() override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;

protected:
    Texture* mStates[eUiButtonState_COUNT];
    Color32 mStatesTint[eUiButtonState_COUNT];
    Point2D mPressInset;
    eUiButtonState mButtonState = eUiButtonState_Normal;
    bool mPressed {};
};

//////////////////////////////////////////////////////////////////////////
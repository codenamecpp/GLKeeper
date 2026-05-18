#pragma once

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

    // Load widget properties from json
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
    void HandleEnableStateChanged() override;
    void HandleVisibilityChanged() override;
    void HandleMouseEnter() override;
    void HandleMouseLeave() override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;

protected:
    Texture* mStates[eUiButtonState_COUNT];
    eUiButtonState mButtonState = eUiButtonState_Normal;
    bool mPressed;
};
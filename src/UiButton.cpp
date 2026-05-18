#include "stdafx.h"
#include "UiButton.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "TextureManager.h"

UiButton::UiButton(): UiButton("button")
{
    memset(mStates, 0, sizeof(mStates));
}

UiButton::UiButton(const std::string& widgetClassName)
    : UiWidget(widgetClassName)
    , mPressed()
{
    memset(mStates, 0, sizeof(mStates));
}


UiButton::UiButton(const UiButton& sourceWidget)
    : UiWidget(sourceWidget)
    , mPressed()
{
    for (int iroller = 0; iroller < eUiButtonState_COUNT; ++iroller)
    {
        mStates[iroller] = sourceWidget.mStates[iroller];
    }
}

UiButton::~UiButton()
{
}

void UiButton::SetButtonStatePicture(eUiButtonState uiButtonState, const std::string& pictureName)
{
    cxx_assert(uiButtonState < eUiButtonState_COUNT);

    Texture* texture = nullptr;

    if (!pictureName.empty())
    {
        TextureManager::LoadParams params;
        params.mConvertNPOT = true; // for ui textures always perform resize
        texture = gTextureManager.GetTexture(pictureName, params);

        cxx_assert(texture);
    }

    mStates[uiButtonState] = texture;
}

void UiButton::Deserialize(const JsonElement& jsonElement)
{
    UiWidget::Deserialize(jsonElement);

    // loading picture property
    auto GetPicturePath = [this, &jsonElement](const std::string& pictureName) -> std::string
        {
            std::string picture_path;
            if (JsonElement stateProperty = jsonElement.FindElement(pictureName))
            {
                picture_path = stateProperty.GetValueString();
            }
            return picture_path;
        };

    SetButtonStatePicture(eUiButtonState_Normal, GetPicturePath("i_state_normal"));
    SetButtonStatePicture(eUiButtonState_Hovered, GetPicturePath("i_state_hovered"));
    SetButtonStatePicture(eUiButtonState_Disabled, GetPicturePath("i_state_disabled"));
    SetButtonStatePicture(eUiButtonState_Pressed, GetPicturePath("i_state_pressed"));

    ButtonStateChanged(); // reset to initial state
}

UiButton* UiButton::CloneSelf() const
{
    UiButton* clone = new UiButton(*this);
    return clone;
}

void UiButton::RenderSelf(UiRenderContext& uiRenderContext)
{
    eUiButtonState stateToDraw = mButtonState;
    if (!mStates[stateToDraw])
    {
        stateToDraw = eUiButtonState_Normal;
    }

    if (mStates[stateToDraw])
    {
        // scale
        Rect2D rcDestination = GetLocalBounds();

        uiRenderContext.DrawTexture(mStates[stateToDraw], COLOR_WHITE, rcDestination);
    }
}

void UiButton::SetPressed(bool isPressed)
{
    bool currentPressed = IsPressed();
    if (currentPressed == isPressed)
        return;

    mPressed = isPressed;
    ButtonStateChanged();
}

void UiButton::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.mButton == MBUTTON_LEFT)
    {
        bool wasClicked = false;
        if (!inputEvent.mPressed)
        {
            wasClicked = IsPressed() && IsScreenPointInsideRect({gInputs.mCursorPositionX, gInputs.mCursorPositionY});
        }
        SetPressed(inputEvent.mPressed);
        if (inputEvent.mPressed)
        {
            gWidgetManager.CaptureFocus(this);
        }
        else
        {
            gWidgetManager.ReleaseFocus(this);
        }

        if (inputEvent.mPressed)
        {
            // notify
            const UiEvent_OnPress eventDesc (MBUTTON_LEFT);
            mEventListeners.IterateListeners([this, &eventDesc](UiEventListener* listener)
                {
                    listener->HandleUiEvent(this, &eventDesc);
                });
        }
        if (wasClicked)
        {
            Click(MBUTTON_LEFT);
        }
        inputEvent.mConsumed = true;
    }
}

void UiButton::ButtonStateChanged()
{
    eUiButtonState forceState = eUiButtonState_Normal;
    if (IsEnabledInHierarchy())
    {
        if (IsHovered())
        {
            forceState = eUiButtonState_Hovered;
        }
        if (IsPressed())
        {
            forceState = eUiButtonState_Pressed;
        }
    }
    else
    {
        forceState = eUiButtonState_Disabled;
    }
    mButtonState = forceState;
}

void UiButton::HandleMouseEnter()
{
    if (IsEnabledInHierarchy())
    {
        ButtonStateChanged();
    }
}

void UiButton::HandleMouseLeave()
{
    if (IsEnabledInHierarchy())
    {
        ButtonStateChanged();
    }
}

void UiButton::HandleEnableStateChanged()
{
    mPressed = 0;
    ButtonStateChanged();
}

void UiButton::HandleVisibilityChanged()
{
    mPressed = 0;
    ButtonStateChanged();
}

void UiButton::Click(int mouseButton)
{
    // notify listeners
    const UiEvent_OnClick eventDesc (mouseButton);
    mEventListeners.IterateListeners([this, &eventDesc](UiEventListener* listener)
        {
            listener->HandleUiEvent(this, &eventDesc);
        });
}
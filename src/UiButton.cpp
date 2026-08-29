#include "stdafx.h"
#include "UiButton.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "TextureManager.h"

UiButton::UiButton(): UiButton("button")
{
}

UiButton::UiButton(const std::string& widgetClassName)
    : UiWidget(widgetClassName)
    , mPressed()
{
    memset(mStates, 0, sizeof(mStates));

    for (Color32& roller: mStatesTint)
    {
        roller = COLOR_WHITE;
    }
}

UiButton::UiButton(const UiButton& sourceWidget)
    : UiWidget(sourceWidget)
    , mPressed()
{
    for (int iroller = 0; iroller < eUiButtonState_COUNT; ++iroller)
    {
        mStates[iroller] = sourceWidget.mStates[iroller];
        mStatesTint[iroller] = sourceWidget.mStatesTint[iroller];
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
    auto InitButtonStatePicture = [this, &jsonElement](eUiButtonState stateId, const std::string& pictureName)
        {
            std::string picture_path;
            if (JsonElement stateProperty = jsonElement.FindElement(pictureName))
            {
                picture_path = stateProperty.GetValueString();
            }
            if (!picture_path.empty())
            {
                this->SetButtonStatePicture(stateId, picture_path);
            }
        };

    InitButtonStatePicture(eUiButtonState_Normal, "i_state_normal");
    InitButtonStatePicture(eUiButtonState_Hovered, "i_state_hovered");
    InitButtonStatePicture(eUiButtonState_Disabled, "i_state_disabled");
    InitButtonStatePicture(eUiButtonState_Pressed, "i_state_pressed");

    // init tint
    auto InitButtonStateTint = [this, &jsonElement](eUiButtonState stateId, const std::string& propName)
        {
            if (JsonElement stateProperty = jsonElement.FindElement(propName))
            {
                if (!JsonReadValue(stateProperty, mStatesTint[stateId]))
                {
                    cxx_assert(false);
                }
            }
        };

    InitButtonStateTint(eUiButtonState_Normal, "tint_normal");
    InitButtonStateTint(eUiButtonState_Hovered, "tint_hovered");
    InitButtonStateTint(eUiButtonState_Disabled, "tint_disabled");
    InitButtonStateTint(eUiButtonState_Pressed, "tint_pressed");

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

        uiRenderContext.DrawTexture(mStates[stateToDraw], mStatesTint[mButtonState], rcDestination);
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
    inputEvent.SetConsumed();

    bool wasClicked = false;

    if (inputEvent.mButton == MBUTTON_LEFT)
    {
        if (!inputEvent.mPressed)
        {
            wasClicked = IsPressed() && IsScreenPointInsideRect(gInputs.GetMousePosition());
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
    }

    if (inputEvent.mPressed)
    {
        const UiEvent_OnPress eventDesc {inputEvent.mButton, inputEvent.mMousePosition};
        mEventListeners.IterateListeners([this, &eventDesc](UiEventListener* listener)
            {
                listener->HandleUiEvent(this, eventDesc);
            });
    }
    else
    {
        const UiEvent_OnRelease eventDesc {inputEvent.mButton, inputEvent.mMousePosition};
        mEventListeners.IterateListeners([this, &eventDesc](UiEventListener* listener)
            {
                listener->HandleUiEvent(this, eventDesc);
            });
    }

    if (wasClicked)
    {
        Click(inputEvent.mButton);
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
            listener->HandleUiEvent(this, eventDesc);
        });
}
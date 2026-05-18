#include "stdafx.h"
#include "UiProductionButton.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "TextureManager.h"

UiProductionButton::UiProductionButton()
    : UiProductionButton("production_button")
{}

UiProductionButton::UiProductionButton(const std::string& widgetClassName)
    : UiButton(widgetClassName)
    , mUnavailable()
    , mSelected()
    , mPicHoveredFrame()
    , mPicSelectedFrame()
{
}

UiProductionButton::UiProductionButton(const UiProductionButton& sourceWidget)
    : UiButton(sourceWidget)
    , mPicHoveredFrame(sourceWidget.mPicHoveredFrame)
    , mPicSelectedFrame(sourceWidget.mPicSelectedFrame)
    , mSelected()
    , mUnavailable()
{
}

UiProductionButton::~UiProductionButton()
{
}

void UiProductionButton::Deserialize(const JsonElement& jsonElement)
{
    UiButton::Deserialize(jsonElement);

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
    // process picture props
    SetHoveredFramePicture(GetPicturePath("i_hovered_frame"));
    SetSelectedFramePicture(GetPicturePath("i_selected_frame"));
}

UiProductionButton* UiProductionButton::CloneSelf() const
{
    UiProductionButton* clone = new UiProductionButton(*this);
    return clone;
}

void UiProductionButton::SetHoveredFramePicture(const std::string& pictureName)
{
    Texture* texture = nullptr;

    if (!pictureName.empty())
    {
        TextureManager::LoadParams params;
        params.mConvertNPOT = true; // for ui textures always perform resize
        texture = gTextureManager.GetTexture(pictureName, params);
        cxx_assert(texture);
    }

    if (mPicHoveredFrame != texture)
    {
        mPicHoveredFrame = texture;
    }
}

void UiProductionButton::SetSelectedFramePicture(const std::string& pictureName)
{
    Texture* texture = nullptr;

    if (!pictureName.empty())
    {
        TextureManager::LoadParams params;
        params.mConvertNPOT = true; // for ui textures always perform resize
        texture = gTextureManager.GetTexture(pictureName, params);

        cxx_assert(texture);
    }

    if (mPicSelectedFrame != texture)
    {
        mPicSelectedFrame = texture;
    }
}

void UiProductionButton::RenderSelf(UiRenderContext& uiRenderContext)
{
    eUiButtonState stateToDraw = mButtonState;
    if (mStates[stateToDraw] == nullptr)
    {
        stateToDraw = eUiButtonState_Normal;
    }

    if (mStates[stateToDraw])
    {
        // scale
        Rect2D rcDestination = GetLocalBounds();

        if (IsPressed())
        {
            rcDestination.x += 1;
            rcDestination.y += 1;
            rcDestination.w -= 2;
            rcDestination.h -= 2;
        }

        uiRenderContext.DrawTexture(mStates[stateToDraw], COLOR_WHITE, rcDestination);
    }

    if (IsUnavailable())
    {
        Rect2D rcDestination = GetLocalBounds();
        uiRenderContext.FillRect(rcDestination, MAKE_RGBA(0x00,0x00,0x00,0x99));
    }

    if (IsSelected())
    {
        if (mPicSelectedFrame)
        {
            // scale
            Rect2D rcDestination = GetLocalBounds();
            uiRenderContext.DrawTexture(mPicSelectedFrame, COLOR_WHITE, rcDestination);
        }
    }

    if (IsHovered())
    {
        if (mPicHoveredFrame)
        {
            // scale
            Rect2D rcDestination = GetLocalBounds();
            uiRenderContext.DrawTexture(mPicHoveredFrame, COLOR_WHITE, rcDestination);
        }
    }
}

void UiProductionButton::SetSelected(bool isSelected)
{
    if (mSelected != isSelected)
    {
        mSelected = isSelected;
    }
}

void UiProductionButton::SetUnavailable(bool isUnavailable)
{
    if (mUnavailable != isUnavailable)
    {
        mUnavailable = isUnavailable;
    }
}
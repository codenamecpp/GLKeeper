#pragma once

#include "UiDefs.h"
#include "UiButton.h"
#include "Texture.h"

//////////////////////////////////////////////////////////////////////////
// Production Button
// Used in hud control panel for selecting rooms, spells, objects etc
//////////////////////////////////////////////////////////////////////////

class UiProductionButton: public UiButton
{
public:
    UiProductionButton();

    // Set picture for button states
    void SetHoveredFramePicture(const std::string& pictureName);
    void SetSelectedFramePicture(const std::string& pictureName);

    // Control button selected state
    virtual void SetSelected(bool isSelected);
    inline void ToggleSelected()
    {
        SetSelected(!mSelected);
    }
    inline bool IsSelected() const { return mSelected; }

    // Control button unavailable state: grayed out
    virtual void SetUnavailable(bool isUnavailable);
    inline void ToggleUnavailable()
    {
        SetUnavailable(!mUnavailable);
    }
    inline bool IsUnavailable() const { return mUnavailable; }

    // Load widget properties from json
    void Deserialize(const JsonElement& jsonElement) override;


protected:
    UiProductionButton(const std::string& widgetClassName);
    UiProductionButton(const UiProductionButton& sourceWidget); // clone props
    ~UiProductionButton();

    UiProductionButton* CloneSelf() const override;

    // override UiWidget
    void RenderSelf(UiRenderContext& uiRenderContext) override;

protected:
    Texture* mPicHoveredFrame = nullptr;
    Texture* mPicSelectedFrame = nullptr;
    bool mSelected;
    bool mUnavailable;
};

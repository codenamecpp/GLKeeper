#pragma once

#include "UiWidget.h"
#include "UiRenderContext.h"

//////////////////////////////////////////////////////////////////////////
// Picture Widget
//////////////////////////////////////////////////////////////////////////

class UiPicture: public UiWidget
{
public:
    UiPicture();

    // Setup texture for picture control
    // @param picturePath: Texture path
    virtual void SetPicture(const std::string& picturePath);

    // Setup stretch mode for picture control
    virtual void SetStretchMode(eUiStretchMode stretchMode);

    // Load widget properties from json
    void Deserialize(const JsonElement& jsonElement) override;

protected:
    UiPicture(const std::string& widgetClassName);
    UiPicture(const UiPicture& sourceWidget); // clone props
    ~UiPicture();

    UiPicture* CloneSelf() const override;

    void RecomptuteCache();
    void InvalidateCache();

    // override UiWidget
    void RenderSelf(UiRenderContext& uiRenderContext) override;
    void HandleSizeChanged(const Point2D& prevSize) override;

protected:
    Texture* mPicTexture = nullptr;
    eUiStretchMode mPicStretchMode = eUiStretchMode_KeepCentered;
    std::vector<Quad2D> mCachedQuads;
    bool mQuadsCacheDirty = true;
};

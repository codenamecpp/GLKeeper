#include "stdafx.h"
#include "UiPicture.h"
#include "UiWidgetManager.h"
#include "TextureManager.h"

UiPicture::UiPicture(): UiPicture("picture")
{
    mInteractive = false;
}

UiPicture::UiPicture(const std::string& widgetClassName) 
    : UiWidget(widgetClassName)
    , mPicTexture()
    , mTintColor(COLOR_WHITE)
{
}

UiPicture::UiPicture(const UiPicture& sourceWidget)
    : UiWidget(sourceWidget)
    , mPicTexture(sourceWidget.mPicTexture)
    , mPicStretchMode(sourceWidget.mPicStretchMode)
    , mTintColor(sourceWidget.mTintColor)
    , mBlendingMode(sourceWidget.mBlendingMode)
{
    InvalidateCache();
}

UiPicture::~UiPicture()
{
}

void UiPicture::SetPicture(const std::string& picturePath)
{
    Texture* uiTexture = nullptr;
    
    if (!picturePath.empty())
    {
        uiTexture = gTextureManager.GetTexture(picturePath, eTextureBacking_Default);
    }

    if (uiTexture != mPicTexture)
    {
        mPicTexture = uiTexture;
        InvalidateCache();
    }
}

void UiPicture::SetStretchMode(eUiStretchMode stretchMode)
{
    if (mPicStretchMode != stretchMode)
    {
        mPicStretchMode = stretchMode;
        InvalidateCache();
    }
}

void UiPicture::Deserialize(const JsonElement& jsonElement)
{
    UiWidget::Deserialize(jsonElement);

    // picture path
    if (JsonElement pathNode = jsonElement.FindElement("path"))
    {
        const std::string pictureName = pathNode.GetValueString();
        if (pictureName.length())
        {
            SetPicture(pictureName);
        }
    }

    // stretch mode
    if (JsonElement stretchNode = jsonElement.FindElement("stretch_mode"))
    {
        const std::string stretchMode = stretchNode.GetValueString();
        bool isSuccess = cxx::parse_enum(stretchMode.c_str(), mPicStretchMode);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Unknown stretch mode '%s'", stretchMode.c_str());
        }
        cxx_assert(isSuccess);
    }

    JsonQuery(jsonElement, "tint", mTintColor);
    
    std::string blendmodeProp;
    if (JsonQuery(jsonElement, "blendmode", blendmodeProp))
    {
        static const std::pair<std::string, eBlendingMode> blendmodes[] =
        {
            {"alpha",       RENDER_STATES_BLENDMODE_ALPHA},
            {"add",         RENDER_STATES_BLENDMODE_ADDITIVE},
            {"alpha_add",   RENDER_STATES_BLENDMODE_ALPHA_ADDITIVE},
            {"multiply",    RENDER_STATES_BLENDMODE_MULTIPLY},
        };
        bool isPropValid = false;
        for (const auto& roller: blendmodes)
        {
            if (blendmodeProp == roller.first)
            {
                mBlendingMode = roller.second;
                isPropValid = true;
                break;
            }
        }
        cxx_assert(isPropValid);
    }

    InvalidateCache();
}

UiPicture* UiPicture::CloneSelf() const
{
    UiPicture* clone = new UiPicture(*this);
    return clone;
}

void UiPicture::RenderSelf(UiRenderContext& uiRenderContext)
{
    if (!mPicTexture)
        return;

    const eBlendingMode prevBlendingMode = uiRenderContext.BeginBlendingMode(mBlendingMode);

    if (mPicStretchMode == eUiStretchMode_Scale || mPicStretchMode == eUiStretchMode_Keep || mPicStretchMode == eUiStretchMode_KeepCentered)
    {
        Rect2D rcDestination;

        switch (mPicStretchMode)
        {
            case eUiStretchMode_Scale:
                rcDestination = GetLocalBounds();
            break;
            case eUiStretchMode_Keep:
            {
                if (mPicTexture)
                {
                    const TextureRegion& textureRegion = mPicTexture->GetTextureRegion();
                    rcDestination.w = textureRegion.mRect.w;
                    rcDestination.h = textureRegion.mRect.h; 
                    rcDestination.x = 0;
                    rcDestination.y = 0;
                }
            }
            break;
            case eUiStretchMode_KeepCentered:
            {
                if (mPicTexture)
                {
                    const TextureRegion& textureRegion = mPicTexture->GetTextureRegion();
                    rcDestination.w = textureRegion.mRect.w;
                    rcDestination.h = textureRegion.mRect.h;
                    rcDestination.x = (mSize.x / 2 - rcDestination.w / 2);
                    rcDestination.y = (mSize.y / 2 - rcDestination.h / 2);
                }
            }
            break;
        }

        uiRenderContext.DrawTexture(mPicTexture, mTintColor, rcDestination);
    }
    else if (mPicStretchMode == eUiStretchMode_TileHorizontal || mPicStretchMode == eUiStretchMode_TileVertical || mPicStretchMode == eUiStretchMode_Tile)
    {
        if (mQuadsCacheDirty)
        {
            mQuadsCacheDirty = false;
            RecomptuteCache();
        }

        const size_t NumQuads = mCachedQuads.size();
        if (NumQuads)
        {
            uiRenderContext.DrawQuads(mPicTexture, &mCachedQuads[0], NumQuads);
        }
    }

    if (prevBlendingMode != mBlendingMode)
    {
        uiRenderContext.BeginBlendingMode(prevBlendingMode);
    }
}

void UiPicture::HandleSizeChanged(const Point2D& prevSize)
{
    InvalidateCache();
}

void UiPicture::RecomptuteCache()
{
    mCachedQuads.clear();

    // size is too small
    if (mSize.x < 1 || mSize.y < 1)
        return;

    cxx_assert(mPicTexture);
    if (!mPicTexture)
        return;

    const TextureRegion& textureRegion = mPicTexture->GetTextureRegion();
    // texture size is too small
    if (textureRegion.mRect.Empty())
        return;

    float coef = 1.0f;
    if (mPicStretchMode == eUiStretchMode_TileHorizontal)
    {
        coef = (mSize.y * 1.0f) / (textureRegion.mRect.h * 1.0f);
    }
    if (mPicStretchMode == eUiStretchMode_TileVertical)
    {
        coef = (mSize.x * 1.0f) / (textureRegion.mRect.w * 1.0f);
    }

    const int TileSize_X = static_cast<int>(textureRegion.mRect.w * coef);
    const int TileSize_Y = static_cast<int>(textureRegion.mRect.h * coef);
    const int ExtraTileSize_X = (mSize.x % TileSize_X);
    const int ExtraTileSize_Y = (mSize.y % TileSize_Y);
    const int NumFullTiles_X = (mSize.x / TileSize_X);
    const int NumFullTiles_Y = (mSize.y / TileSize_Y);
    const int ExtraTexturesPixels_X = static_cast<int>((mSize.x % TileSize_X) / coef);
    const int ExtraTexturesPixels_Y = static_cast<int>((mSize.y % TileSize_Y) / coef);

    // num full tiles plus partial tile
    const int NumTiles_X = NumFullTiles_X + (ExtraTexturesPixels_X > 0 ? 1 : 0);
    const int NumTiles_Y = NumFullTiles_Y + (ExtraTexturesPixels_Y > 0 ? 1 : 0);

    if (NumFullTiles_X < 1 && NumFullTiles_Y < 1 && ExtraTexturesPixels_X < 1 && ExtraTexturesPixels_Y < 1)
        return;

    // allocate quads
    mCachedQuads.resize(NumTiles_X * NumTiles_Y);

    if (mPicStretchMode == eUiStretchMode_Tile)
    {
        for (int currentY = 0; currentY < NumTiles_Y; ++currentY)
        for (int currentX = 0; currentX < NumTiles_X; ++currentX)
        {
            const int CurrentTilePixels_X = (currentX == NumFullTiles_X) ? ExtraTexturesPixels_X : textureRegion.mRect.w;
            const int CurrentTilePixels_Y = (currentY == NumFullTiles_Y) ? ExtraTexturesPixels_Y : textureRegion.mRect.h;
            const Rect2D rcSrc 
            {
                0, 0, CurrentTilePixels_X, CurrentTilePixels_Y
            };
            const Rect2D rcDest 
            {
                currentX * textureRegion.mRect.w, 
                currentY * textureRegion.mRect.h, 
                CurrentTilePixels_X, 
                CurrentTilePixels_Y
            };
            mCachedQuads[currentY * NumTiles_X + currentX].BuildTextureQuad(textureRegion, rcSrc, rcDest, mTintColor);
        }
    }
    else if (mPicStretchMode == eUiStretchMode_TileHorizontal)
    {
        for (int currentTile = 0; currentTile < NumTiles_X; ++currentTile)
        {
            const bool isExtraTile = (currentTile == NumFullTiles_X);
            const Rect2D rcSrc 
            { 
                0, 0, (isExtraTile ? ExtraTexturesPixels_X : textureRegion.mRect.w), textureRegion.mRect.h 
            };
            const Rect2D rcDest 
            {
                currentTile * TileSize_X, 0, 
                isExtraTile ? ExtraTileSize_X : TileSize_X, TileSize_Y
            };
            mCachedQuads[currentTile].BuildTextureQuad(textureRegion, rcSrc, rcDest, mTintColor);
        }
    }
    else if (mPicStretchMode == eUiStretchMode_TileVertical)
    {
        for (int currentTile = 0; currentTile < NumTiles_Y; ++currentTile)
        {
            const bool isExtraTile = (currentTile == NumFullTiles_Y);
            const Rect2D rcSrc 
            { 
                0, 0, textureRegion.mRect.w, (isExtraTile ? ExtraTexturesPixels_Y : textureRegion.mRect.h) 
            };
            const Rect2D rcDest 
            {
                0, currentTile * TileSize_Y, 
                TileSize_X, isExtraTile ? ExtraTileSize_Y : TileSize_Y
            };
            mCachedQuads[currentTile].BuildTextureQuad(textureRegion, rcSrc, rcDest, mTintColor);
        }
    } // if
}

void UiPicture::InvalidateCache()
{
    mQuadsCacheDirty = true;
}

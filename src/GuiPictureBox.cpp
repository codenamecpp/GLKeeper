#include "pch.h"
#include "GuiPictureBox.h"
#include "GuiRenderer.h"
#include "Texture2D.h"

// widget class factory
static GuiWidgetFactory<GuiPictureBox> _PictureBoxWidgetsFactory;

GuiWidgetClass gPictureBoxWidgetClass("picture_box", &_PictureBoxWidgetsFactory, &gBaseWidgetClass);

//////////////////////////////////////////////////////////////////////////

GuiPictureBox::GuiPictureBox() : GuiPictureBox(&gPictureBoxWidgetClass)
{
}

GuiPictureBox::GuiPictureBox(GuiWidgetClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiPictureBox::GuiPictureBox(GuiPictureBox* copyWidget)
    : GuiWidget(copyWidget)
    , mSizeMode(copyWidget->mSizeMode)
    , mTexture(copyWidget->mTexture)
    , mQuadsCache(copyWidget->mQuadsCache)
{
}

void GuiPictureBox::SetSizeMode(eGuiSizeMode sizeMode)
{
    if (mSizeMode == sizeMode)
        return;

    mSizeMode = sizeMode;
    InvalidateCache();
}

void GuiPictureBox::SetTexture(Texture2D* texture)
{
    if (mTexture == texture)
        return;

    mTexture = texture;
    InvalidateCache();
}

GuiPictureBox* GuiPictureBox::ConstructClone()
{
    GuiPictureBox* selfClone = new GuiPictureBox(this);
    return selfClone;
}

void GuiPictureBox::HandleRender(GuiRenderer& renderContext)
{
    if (mTexture == nullptr)
        return;

    if (mQuadsCache.empty())
    {
        GenerateQuads();

        if (mQuadsCache.empty()) // cannot generate quads
            return;
    }

    renderContext.DrawQuads(mTexture, 
        mQuadsCache.data(), 
        mQuadsCache.size());
}

void GuiPictureBox::HandleSizeChanged(const Point& prevSize)
{
    InvalidateCache();
}

void GuiPictureBox::InvalidateCache()
{
    mQuadsCache.clear();
}

void GuiPictureBox::GenerateQuads()
{
    debug_assert(mTexture);
    if (!mTexture->IsTextureLoaded())
    {
        mTexture->LoadTexture();
    }

    Rectangle rcDestination = GetLocalRect();
    if (rcDestination.w < 1 || rcDestination.h < 1)
        return;

    Color32 verticesColor = Color32_White;

    const Point& imageSize = mTexture->mTextureDesc.mImageDimensions;
    debug_assert(imageSize.x > 0 && imageSize.y > 0);

    if (mSizeMode == eGuiSizeMode_Scale || mSizeMode == eGuiSizeMode_Keep || 
        mSizeMode == eGuiSizeMode_KeepCentered || mSizeMode == eGuiSizeMode_ProportionalScale)
    {
        switch (mSizeMode)
        {
            case eGuiSizeMode_ProportionalScale:
            {
                float scalex = (rcDestination.w * 1.0f) / (imageSize.x * 1.0f);
                float scaley = (rcDestination.h * 1.0f) / (imageSize.y * 1.0f);
                float scaleValue = glm::min(scalex, scaley);

                rcDestination.w = (int)(imageSize.x * scaleValue);
                rcDestination.h = (int)(imageSize.y * scaleValue);
            }
            break;
            case eGuiSizeMode_Keep:
                rcDestination.w = imageSize.x;
                rcDestination.h = imageSize.y;
                rcDestination.x = 0;
                rcDestination.y = 0;
            break;
            case eGuiSizeMode_KeepCentered:
                rcDestination.w = imageSize.x;
                rcDestination.h = imageSize.y;
                rcDestination.x = (mSize.x / 2 - rcDestination.w / 2);
                rcDestination.y = (mSize.y / 2 - rcDestination.h / 2);
            break;
        }

        mQuadsCache.emplace_back();

        GuiQuadStruct& quad = mQuadsCache.back();
        quad.SetupVertices(mTexture, rcDestination, verticesColor);
    }
    else if (mSizeMode == eGuiSizeMode_TileHorizontal || mSizeMode == eGuiSizeMode_TileVertical || mSizeMode == eGuiSizeMode_Tile)
    {
        float coef = 1.0f;
        if (mSizeMode == eGuiSizeMode_TileHorizontal)
        {
            coef = (mSize.y * 1.0f) / (imageSize.y * 1.0f);
        }
        if (mSizeMode == eGuiSizeMode_TileVertical)
        {
            coef = (mSize.x * 1.0f) / (imageSize.x * 1.0f);
        }

        const int TileSize_X = (int) (imageSize.x * coef);
        const int TileSize_Y = (int) (imageSize.y * coef);
        const int ExtraTileSize_X = (mSize.x % TileSize_X);
        const int ExtraTileSize_Y = (mSize.y % TileSize_Y);
        const int NumFullTiles_X = (mSize.x / TileSize_X);
        const int NumFullTiles_Y = (mSize.y / TileSize_Y);
        const int ExtraTexturesPixels_X = (int) ((mSize.x % TileSize_X) / coef);
        const int ExtraTexturesPixels_Y = (int) ((mSize.y % TileSize_Y) / coef);

        // num full tiles plus partial tile
        const int NumTiles_X = NumFullTiles_X + (ExtraTexturesPixels_X > 0 ? 1 : 0);
        const int NumTiles_Y = NumFullTiles_Y + (ExtraTexturesPixels_Y > 0 ? 1 : 0);

        if (NumFullTiles_X < 1 && NumFullTiles_Y < 1 && ExtraTexturesPixels_X < 1 && ExtraTexturesPixels_Y < 1)
            return;

        // allocate quads
        mQuadsCache.resize(NumTiles_X * NumTiles_Y);

        if (mSizeMode == eGuiSizeMode_Tile)
        {
            for (int currentY = 0; currentY < NumTiles_Y; ++currentY)
            for (int currentX = 0; currentX < NumTiles_X; ++currentX)
            {
                const int CurrentTilePixels_X = (currentX == NumFullTiles_X) ? ExtraTexturesPixels_X : imageSize.x;
                const int CurrentTilePixels_Y = (currentY == NumFullTiles_Y) ? ExtraTexturesPixels_Y : imageSize.y;
                const Rectangle rcSrc 
                {
                    0, 0, CurrentTilePixels_X, CurrentTilePixels_Y
                };
                const Rectangle rcDest 
                {
                    currentX * imageSize.x, 
                    currentY * imageSize.y, 
                    CurrentTilePixels_X, 
                    CurrentTilePixels_Y
                };
                mQuadsCache[currentY * NumTiles_X + currentX].SetupVertices(mTexture, rcSrc, rcDest, verticesColor);
            }
        }
        else if (mSizeMode == eGuiSizeMode_TileHorizontal)
        {
            for (int currentTile = 0; currentTile < NumTiles_X; ++currentTile)
            {
                const bool isExtraTile = (currentTile == NumFullTiles_X);
                const Rectangle rcSrc 
                { 
                    0, 0, (isExtraTile ? ExtraTexturesPixels_X : imageSize.x), imageSize.y 
                };
                const Rectangle rcDest 
                {
                    currentTile * TileSize_X, 0, 
                    isExtraTile ? ExtraTileSize_X : TileSize_X, TileSize_Y
                };
                mQuadsCache[currentTile].SetupVertices(mTexture, rcSrc, rcDest, verticesColor);
            }
        }
        else if (mSizeMode == eGuiSizeMode_TileVertical)
        {
            for (int currentTile = 0; currentTile < NumTiles_Y; ++currentTile)
            {
                const bool isExtraTile = (currentTile == NumFullTiles_Y);
                const Rectangle rcSrc 
                { 
                    0, 0, imageSize.x, (isExtraTile ? ExtraTexturesPixels_Y : imageSize.y) 
                };
                const Rectangle rcDest 
                {
                    0, currentTile * TileSize_Y, 
                    TileSize_X, isExtraTile ? ExtraTileSize_Y : TileSize_Y
                };
                mQuadsCache[currentTile].SetupVertices(mTexture, rcSrc, rcDest, verticesColor);
            }
        } // if
    }
    else
    {
        debug_assert(false);
    }
}

#include "pch.h"
#include "GuiPictureBox.h"
#include "GuiRenderer.h"
#include "Texture2D.h"

// base widget class factory
static GuiWidgetFactory<GuiPictureBox> _PictureBoxWidgetsFactory;

GuiWidgetClass gPictureBoxWidgetClass("picture_box", &_PictureBoxWidgetsFactory);

//////////////////////////////////////////////////////////////////////////

GuiPictureBox::GuiPictureBox() : GuiPictureBox(&gPictureBoxWidgetClass)
{
}

GuiPictureBox::GuiPictureBox(GuiWidgetClass* widgetClass)
{
}

GuiPictureBox::GuiPictureBox(GuiPictureBox* copyWidget)
    : GuiWidget(copyWidget)
    , mPictureStretch(copyWidget->mPictureStretch)
    , mTexture(copyWidget->mTexture)
    , mQuadsCache(copyWidget->mQuadsCache)
{
}

void GuiPictureBox::SetStretchMode(eGuiStretchMode stretchMode)
{
    if (mPictureStretch == stretchMode)
        return;

    mPictureStretch = stretchMode;
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
    GuiPictureBox* selfCopy = new GuiPictureBox(this);
    return selfCopy;
}

void GuiPictureBox::HandleRenderSelf(GuiRenderer& renderContext)
{
    if (mTexture == nullptr)
        return;

    if (mQuadsCache.empty())
    {
        GenerateQuads();

        if (mQuadsCache.empty()) // cannot generate quads
            return;
    }

    Rect2D localRect;
    GetLocalRect(localRect);
    renderContext.FillRect(localRect, Color32_Yellow);

    renderContext.DrawQuads(mTexture, 
        mQuadsCache.data(), 
        mQuadsCache.size());
}

void GuiPictureBox::HandleSizeChanged(const Size2D& prevSize)
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

    const Size2D& imageSize = mTexture->mTextureDesc.mImageDimensions;

    if (mPictureStretch == eGuiStretchMode_Scale || mPictureStretch == eGuiStretchMode_Keep || 
        mPictureStretch == eGuiStretchMode_KeepCentered || mPictureStretch == eGuiStretchMode_ProportionalScale)
    {
        Rect2D rcDestination;
        GetLocalRect(rcDestination);

        if (rcDestination.mSizeX < 1 || rcDestination.mSizeY < 1)
            return;

        switch (mPictureStretch)
        {
            case eGuiStretchMode_ProportionalScale:
            {
                float scalex = (rcDestination.mSizeX * 1.0f) / (imageSize.x * 1.0f);
                float scaley = (rcDestination.mSizeY * 1.0f) / (imageSize.y * 1.0f);
                float scaleValue = glm::min(scalex, scaley);

                rcDestination.mSizeX = (int)(imageSize.x * scaleValue);
                rcDestination.mSizeY = (int)(imageSize.y * scaleValue);
            }
            break;
            case eGuiStretchMode_Keep:
                rcDestination.mSizeX = imageSize.x;
                rcDestination.mSizeY = imageSize.y;
                rcDestination.mX = 0;
                rcDestination.mY = 0;
            break;
            case eGuiStretchMode_KeepCentered:
                rcDestination.mSizeX = imageSize.x;
                rcDestination.mSizeY = imageSize.y;
                rcDestination.mX = (mSize.x / 2 - rcDestination.mSizeX / 2);
                rcDestination.mY = (mSize.y / 2 - rcDestination.mSizeY / 2);
            break;
        }

        mQuadsCache.emplace_back();

        GuiQuadStruct& quad = mQuadsCache.back();
        quad.SetupVertices(mTexture, rcDestination, Color32_White);
    }
    else if (mPictureStretch == eGuiStretchMode_TileHorizontal || mPictureStretch == eGuiStretchMode_TileVertical || mPictureStretch == eGuiStretchMode_Tile)
    {
        // todo
    }
    else
    {
        // todo
        debug_assert(false);
    }
}

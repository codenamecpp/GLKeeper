#include "stdafx.h"
#include "TextureAtlas.h"

//////////////////////////////////////////////////////////////////////////

static constexpr int MaxAtlasTextureMipsCount = 3;
static constexpr int MaxTextureAtlasPadding = 8;

//////////////////////////////////////////////////////////////////////////

void TextureAtlas::GuillotinePacker::Setup(const Point2D& rectSize, int padding, int alignment)
{
    mFreeRects.reserve(1024);
    mFreeRects.push_back({0, 0, rectSize.x, rectSize.y});
    mAlignment = std::max(alignment, 1);
    mPadding = std::max(padding, 0);
    cxx_assert(mPadding == padding);
    cxx_assert(mAlignment == alignment);
}

bool TextureAtlas::GuillotinePacker::AllocateRect(const Point2D& rectSize, Rect2D& outRect)
{
    int candidateIndex = -1;
    int candidateMinArea = INT_MAX;

    const Point2D rectSizeWithPadding { rectSize.x + mPadding * 2, rectSize.y + mPadding * 2 };

    const int aligned_w = (mAlignment > 1) ? cxx::align_up(rectSizeWithPadding.x, mAlignment) : rectSizeWithPadding.x;
    const int aligned_h = (mAlignment > 1) ? cxx::align_up(rectSizeWithPadding.y, mAlignment) : rectSizeWithPadding.y;

    // best area fit
    int rectsCounter = 0;
    for (const Rect2D& currRect: mFreeRects)
    {
        if ((aligned_w <= currRect.w) && 
            (aligned_h <= currRect.h))
        {
            const int area = currRect.w * currRect.h;
            if (area < candidateMinArea)
            {
                candidateMinArea = area;
                candidateIndex = rectsCounter;
            }
        }
        ++rectsCounter;
    }

    if (candidateIndex == -1)
        return false;

    const Rect2D targetRect = mFreeRects[candidateIndex];
    const Rect2D placedRect { targetRect.x, targetRect.y, aligned_w, aligned_h };

    // set result rect
    outRect.x = placedRect.x + mPadding;
    outRect.y = placedRect.y + mPadding;
    outRect.w = rectSize.x;
    outRect.h = rectSize.y;

    mFreeRects.erase(mFreeRects.begin() + candidateIndex);
    SplitFreeRect(targetRect, placedRect);
    ++mAllocated;
    return true;
}

void TextureAtlas::GuillotinePacker::SplitFreeRect(const Rect2D& freeRect, const Rect2D& placed)
{
    const int extentW = freeRect.w - placed.w;
    const int extentH = freeRect.h - placed.h;

    if ((extentW <= 0) && (extentH <= 0))
        return;

    /*
        free rect:
             _________________________
            |        |                |
            | placed |    right       |
            |________|_ _ _ _ _ _ _ _ |
            |                         |
            |       bottom            |
            |_________________________|
    */

    const Rect2D rcRight {
        freeRect.x + placed.w, 
        freeRect.y, 
        extentW, 
        (extentW > extentH) ? freeRect.h : placed.h };

    if (!rcRight.Empty())
    {
        mFreeRects.push_back(rcRight);
    }

    const Rect2D rcBottom {
        freeRect.x, 
        freeRect.y + placed.h, 
        (extentW > extentH) ? placed.w : freeRect.w, 
        extentH };

    if (!rcBottom.Empty())
    {
        mFreeRects.push_back(rcBottom);
    }
}

void TextureAtlas::GuillotinePacker::Clear()
{
    mFreeRects.clear();
    mAllocated = {};
}

//////////////////////////////////////////////////////////////////////////

TextureAtlas::TextureAtlas(TextureSourceId textureSourceId)
    : mTextureSourceId(textureSourceId)
{
    cxx_assert(mTextureSourceId > 0);
}

bool TextureAtlas::Setup(const Point2D& textureDimensions, ePixelFormat pixelFormat, int numMipmaps)
{
    Purge();

    cxx_assert(numMipmaps > 0);
    cxx_assert((textureDimensions.x >= 0) && cxx::is_pot(textureDimensions.x));
    cxx_assert((textureDimensions.y >= 0) && cxx::is_pot(textureDimensions.y));
    cxx_assert(pixelFormat != ePixelFormat_Null);

    if ((pixelFormat == ePixelFormat_Null) || (numMipmaps < 1))
        return false;

    mMipMapsCount = std::min(numMipmaps, MaxAtlasTextureMipsCount);

    mTextureDimensions = textureDimensions;

    const int padding = std::min(MaxTextureAtlasPadding, (1 << (mMipMapsCount - 1)));
    const int alignment = 1 << (mMipMapsCount - 1);

    mRectsPacker.Setup({mTextureDimensions.x, mTextureDimensions.y}, padding, alignment);

    mPixelFormat = pixelFormat;
    return true;
}

void TextureAtlas::Purge()
{
    mRectsPacker.Clear();
    mUpdateRects.clear();
    mGpuTextureResource.reset();
    mTextureDimensions = {};
    mPixelFormat = ePixelFormat_Null;
    mMipMapsCount = 0;
}

void TextureAtlas::UpdateRenderData()
{
    if (mUpdateRects.empty())
        return;

    GpuTexture2D::ScopedBinder scopedBinder{mGpuTextureResource.get()};

    const int padding = mRectsPacker.GetPadding();

    BitmapImage bi;
    for (auto& roller: mUpdateRects)
    {
        const Point2D baseWH = roller.first.GetSize();
        const Point2D baseXY = roller.first.GetPosition();
        const Point2D placedXY
        {
            baseXY.x - padding,
            baseXY.y - padding
        };

        for (int imip = 0; imip < mMipMapsCount; ++imip)
        {
            const int mipmapPadding = (padding >> imip);
            bi.CreateFrom(roller.second, imip, mipmapPadding);

            const Point2D uploadXY
            {
                placedXY.x >> imip,
                placedXY.y >> imip
            };
            mGpuTextureResource->Upload(uploadXY, bi.GetDimensions(), bi.GetMipPixels(0), imip);
        }
    }

    mUpdateRects.clear();
}

bool TextureAtlas::TryAppendTexture(BitmapImage& bitmapImage, TextureRegion& region)
{
    if (!bitmapImage.HasContent() || 
        !bitmapImage.HasPixelFormat(mPixelFormat) ||
        (bitmapImage.GetMipsCount() < mMipMapsCount))
    {
        return false;
    }

    Point2D imageDims = bitmapImage.GetDimensions();
    Rect2D rect;

    bool isSuccess = mRectsPacker.AllocateRect(imageDims, rect);
    if (isSuccess)
    {
        region.Init(rect, mTextureDimensions);
        // queue update
        auto& updateRect = mUpdateRects.emplace_back();
        updateRect.first = region.mRect;
        updateRect.second.Swap(bitmapImage);
    }
    return isSuccess;
}

void TextureAtlas::BindTexture(eTextureUnit textureUnit)
{
    GpuTexture2D* gpuTexture = GetRenderResource();
    cxx_assert(gpuTexture && mUpdateRects.empty());
    gRenderDevice.BindTexture2D(textureUnit, gpuTexture);
}

void TextureAtlas::EnsureRenderDataInited()
{
    if (mGpuTextureResource)
        return;

    mGpuTextureResource = gRenderDevice.CreateTexture2D();
    cxx_assert(mGpuTextureResource);

    cxx::static_vector<Texture2DMip, MaxAtlasTextureMipsCount> textureMips;
    for (int icounter = 0; icounter < mMipMapsCount; ++icounter)
    {
        Texture2DMip mipData;
        mipData.mPixelsData = nullptr;
        mipData.mSizex = mTextureDimensions.x >> (1 * icounter);
        mipData.mSizey = mTextureDimensions.y >> (1 * icounter);
        textureMips.push_back(mipData);
    }

    bool isSuccess = mGpuTextureResource->Create(textureMips.size(), textureMips.data(), mPixelFormat);
    cxx_assert(isSuccess);
    if (isSuccess)
    {
        mGpuTextureResource->SetSamplerState(eTextureFiltering_Trilinear, eTextureRepeating_ClampToEdge);
        return;
    }

    gConsole.LogMessage(eLogLevel_Error, "Cannot allocate texture atlas render data");
    Purge();
}

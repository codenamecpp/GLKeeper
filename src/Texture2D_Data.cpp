#include "pch.h"
#include "Texture2D_Data.h"
#include "stb_image_write.h"

Texture2D_Data::Texture2D_Data()
{
    Clear();
}

Texture2D_Data::~Texture2D_Data()
{
    Clear();
}

void Texture2D_Data::Setup(eTextureFormat format, const Size2D& dimensions, bool transparent, const unsigned char* sourceData)
{
    Clear();

    if (format == eTextureFormat_Null)
    {
        debug_assert(false);
        return;
    }

    if (dimensions.x < 1 || dimensions.y < 1)
    {
        debug_assert(false);
        return;
    }

    mSize = dimensions;
    mImageSize = dimensions;
    mTransparent = transparent;
    mPixelsFormat = format;

    const int dataLength = GetTextureFormatBytesCount(format) * dimensions.x * dimensions.y;
    if (sourceData)
    {
        mBitmap.assign(sourceData, sourceData + dataLength);
    }
    else
    {
        mBitmap.resize(dataLength);
    }
}

bool Texture2D_Data::DumpToFile(const std::string& filePath)
{
    if (IsNull())
    {
        debug_assert(false);
        return false;
    }

    int comp = GetTextureFormatBytesCount(mPixelsFormat);
    return stbi_write_png(filePath.c_str(), mSize.x, mSize.y, comp, mBitmap.data(), mSize.x * comp) > 0;
}

void Texture2D_Data::SetupMipmaps(int newMipsCount)
{
    if (IsNull() || NonPOT())
    {
        debug_assert(false);
        return;
    }

    int currMipsCount = GetMipmapsCount();

    if (currMipsCount == newMipsCount)
        return;

    if (newMipsCount < 1)
    {
        ClearMipmaps();
        return;
    }

    if (currMipsCount > newMipsCount)
    {
        mMipmaps.resize(newMipsCount);
        return;
    }

    // add new mips
    mMipmaps.reserve(newMipsCount);
    for (int icurr = currMipsCount; icurr < newMipsCount; ++icurr)
    {
        int sizex = GetTextureMipmapDimensions(mSize.x, icurr + 1);
        int sizey = GetTextureMipmapDimensions(mSize.y, icurr + 1);
        const int dataLength = GetTextureFormatBytesCount(mPixelsFormat) * sizex * sizey;
        if (dataLength == 0)
        {
            debug_assert(false);
            return;
        }
        mMipmaps.emplace_back();
        mMipmaps[icurr].mBitmap.resize(dataLength);
    }
}

void Texture2D_Data::ClearMipmaps()
{
    mMipmaps.clear();
}

void Texture2D_Data::ResizeToPOT(bool freeCurrent)
{
    if (IsNull())
    {
        debug_assert(false);
        return;
    }

    if (IsPOT())
    {
        // nothing to do, pot already
        return;
    }

    // get nearest pot dimensions
    Size2D newTextureSize;
    newTextureSize.x = cxx::get_next_pot(mSize.x);
    newTextureSize.y = cxx::get_next_pot(mSize.y);
    
    mImageSize = mSize;
    mSize = newTextureSize;

    mMaxU = (mImageSize.x * 1.0f) / (newTextureSize.x * 1.0f);
    mMaxV = (mImageSize.y * 1.0f) / (newTextureSize.y * 1.0f);

    const int bpp = GetTextureFormatBytesCount(mPixelsFormat);
    const int dataLength = bpp * newTextureSize.x * newTextureSize.y;
    
    if (freeCurrent)
    {
        mBitmap.resize(dataLength);
        return;
    }

    const int srcRowLength = bpp * mImageSize.x;
    const int dstDowLength = bpp * newTextureSize.x;

    // copy old content
    ByteArray temporaryBuffer;
    temporaryBuffer.resize(dataLength);

    for (int currenty = 0; currenty < mImageSize.y; ++currenty)
    {
        int srcOffset = currenty * srcRowLength; 
        int dstOffset = currenty * dstDowLength;
        ::memcpy(temporaryBuffer.data() + dstOffset, mBitmap.data() + srcOffset, srcRowLength);

        // put additional pixel to right
        srcOffset += srcRowLength - bpp;
        dstOffset += srcRowLength;
        ::memcpy(temporaryBuffer.data() + dstOffset, mBitmap.data() + srcOffset, bpp);
    }
    // put additional line bottom
    int srcOffset = (mImageSize.y - 1) * dstDowLength;
    int dstOffset = mImageSize.y * dstDowLength;
    ::memcpy(temporaryBuffer.data() + dstOffset, temporaryBuffer.data() + srcOffset, srcRowLength + bpp);

    mBitmap.swap(temporaryBuffer);
}

void Texture2D_Data::Clear()
{
    mPixelsFormat = eTextureFormat_Null;
    mBitmap.clear();
    mMipmaps.clear();
    mSize.x = 0;
    mSize.y = 0;
    mImageSize.x = 0;
    mImageSize.y = 0;
    mMaxU = 1.0f;
    mMaxV = 1.0f;
    mTransparent = false;
}

void Texture2D_Data::SwapContent(Texture2D_Data& sourceData)
{
    std::swap(sourceData.mBitmap, mBitmap);
    std::swap(sourceData.mMipmaps, mMipmaps);
    std::swap(sourceData.mSize, mSize);
    std::swap(sourceData.mImageSize, mImageSize);
    std::swap(sourceData.mMaxU, mMaxU);
    std::swap(sourceData.mMaxV, mMaxV);
    std::swap(sourceData.mTransparent, mTransparent);
    std::swap(sourceData.mPixelsFormat, mPixelsFormat);
}

bool Texture2D_Data::IsNull() const
{
    return (mPixelsFormat == eTextureFormat_Null) || 
        (mSize.x == 0 || mSize.y == 0) || mBitmap.empty();
}

bool Texture2D_Data::NonNull() const
{
    return !IsNull();
}

bool Texture2D_Data::IsPOT() const
{
    bool wpot = cxx::is_pot(mSize.x);
    bool hpot = cxx::is_pot(mSize.y);

    return wpot && hpot;
}

bool Texture2D_Data::NonPOT() const
{
    return !IsPOT();
}

int Texture2D_Data::GetMipmapsCount() const
{
    // not counting primary bitmap
    return (int) mMipmaps.size();
}

bool Texture2D_Data::HasMipmaps() const
{
    return !mMipmaps.empty();
}

bool Texture2D_Data::FillWithCheckerBoard()
{
    if (IsNull())
        return false;

    if (mPixelsFormat == eTextureFormat_R8 || mPixelsFormat == eTextureFormat_R8UI)
    {
        for (int iy = 0; iy < mSize.y; ++iy)
            for (int ix = 0; ix < mSize.x; ++ix)
            {
                unsigned char color = 0xFF;
                if ((iy / 8) % 2 == (ix / 8) % 2)
                {
                    color = 0x00;
                }
                int offset = (iy * mSize.x) + (ix);
                mBitmap[offset] = color;
            }

        return true; 
    }

    if (mPixelsFormat == eTextureFormat_RGB8)
    {
        for (int iy = 0; iy < mSize.y; ++iy)
            for (int ix = 0; ix < mSize.x; ++ix)
            {
                unsigned char color = 0xFF;
                if ((iy / 8) % 2 == (ix / 8) % 2)
                {
                    color = 0x00;
                }
                int offset = (iy * mSize.x * 3) + (ix * 3);
                mBitmap[offset + 0] = color;
                mBitmap[offset + 1] = 0x00;
                mBitmap[offset + 2] = color;
            }

        return true;
    }

    if (mPixelsFormat == eTextureFormat_RGBA8 || mPixelsFormat == eTextureFormat_RGBA8UI)
    {
        for (int iy = 0; iy < mSize.y; ++iy)
            for (int ix = 0; ix < mSize.x; ++ix)
            {
                unsigned char color = 0xFF;
                if ((iy / 8) % 2 == (ix / 8) % 2)
                {
                    color = 0x00;
                }
                int offset = (iy * mSize.x * 4) + (ix * 4);
                mBitmap[offset + 0] = color;
                mBitmap[offset + 1] = 0x00;
                mBitmap[offset + 2] = color;
                mBitmap[offset + 3] = 0xFF;
            }

        return true;
    }
    return false;
}

bool Texture2D_Data::FillWithColor(Color32 color)
{
    if (IsNull())
        return false;

    if (mPixelsFormat == eTextureFormat_R8 || mPixelsFormat == eTextureFormat_R8UI)
    {
        for (int iy = 0; iy < mSize.y; ++iy)
            for (int ix = 0; ix < mSize.x; ++ix)
            {
                int offset = (iy * mSize.x) + (ix);
                mBitmap[offset] = color.mR;
            }

        return true; 
    }

    if (mPixelsFormat == eTextureFormat_RGB8)
    {
        for (int iy = 0; iy < mSize.y; ++iy)
            for (int ix = 0; ix < mSize.x; ++ix)
            {
                int offset = (iy * mSize.x * 3) + (ix * 3);
                mBitmap[offset + 0] = color.mR;
                mBitmap[offset + 1] = color.mG;
                mBitmap[offset + 2] = color.mB;
            }

        return true;
    }

    if (mPixelsFormat == eTextureFormat_RGBA8 || mPixelsFormat == eTextureFormat_RGBA8UI)
    {
        for (int iy = 0; iy < mSize.y; ++iy)
            for (int ix = 0; ix < mSize.x; ++ix)
            {
                int offset = (iy * mSize.x * 4) + (ix * 4);
                mBitmap[offset + 0] = color.mR;
                mBitmap[offset + 1] = color.mG;
                mBitmap[offset + 2] = color.mB;
                mBitmap[offset + 3] = color.mA;
            }

        return true;
    }
    return false;
}

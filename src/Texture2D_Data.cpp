#include "pch.h"
#include "Texture2D_Data.h"
#include "stb_image_write.h"

Texture2D_Data::Texture2D_Data()
{
}

Texture2D_Data::~Texture2D_Data()
{
    Clear();
}

void Texture2D_Data::Setup(eTextureFormat format, int sizex, int sizey, bool transparent, const unsigned char* sourceData)
{
    Clear();

    if (format == eTextureFormat_Null)
    {
        debug_assert(false);
        return;
    }

    if (sizex < 1 || sizex < 1)
    {
        debug_assert(false);
        return;
    }

    mDimsW = sizex;
    mDimsH = sizey;
    mTransparent = transparent;
    mPixelsFormat = format;

    const int dataLength = GetTextureFormatBytesCount(format) * sizex * sizey;
    if (sourceData)
    {
        mBitmap.assign(sourceData, sourceData + dataLength);
    }
    else
    {
        mBitmap.resize(dataLength);
    }
}

bool Texture2D_Data::SaveToFile(const std::string& filePath)
{
    if (IsNull())
    {
        debug_assert(false);
        return false;
    }

    int comp = GetTextureFormatBytesCount(mPixelsFormat);
    return stbi_write_png(filePath.c_str(), mDimsW, mDimsH, comp, mBitmap.data(), mDimsW * comp) > 0;
}

bool Texture2D_Data::SaveToFileMip(const std::string& filePath, int mipIndex)
{
    if (IsNull())
    {
        debug_assert(false);
        return false;
    }

    int currMipsCount = GetMipmapsCount();

    debug_assert(currMipsCount > mipIndex);
    int comp = GetTextureFormatBytesCount(mPixelsFormat);
    int mipSizex = GetTextureMipmapDimensions(mDimsW, mipIndex + 1);
    int mipSizey = GetTextureMipmapDimensions(mDimsH, mipIndex + 1);

    return stbi_write_png(filePath.c_str(), mipSizex, mipSizey, comp, mMipmaps[mipIndex].mBitmap.data(), mipSizex * comp) > 0;
}

void Texture2D_Data::SetupMipmaps(int newMipsCount)
{
    if (IsNull())
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
        int sizex = GetTextureMipmapDimensions(mDimsW, icurr + 1);
        int sizey = GetTextureMipmapDimensions(mDimsH, icurr + 1);
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

void Texture2D_Data::Clear()
{
    mBitmap.clear();
    mMipmaps.clear();
    mDimsH = 0;
    mDimsW = 0;
    mTransparent = false;
    mPixelsFormat = eTextureFormat_Null;
}

void Texture2D_Data::SwapContent(Texture2D_Data& sourceData)
{
    std::swap(sourceData.mBitmap, mBitmap);
    std::swap(sourceData.mMipmaps, mMipmaps);
    std::swap(sourceData.mDimsW, mDimsW);
    std::swap(sourceData.mDimsH, mDimsH);
    std::swap(sourceData.mPixelsFormat, mPixelsFormat);
}

bool Texture2D_Data::IsNull() const
{
    return mDimsH == 0 && mDimsW == 0;
}

bool Texture2D_Data::IsPOT() const
{
    return cxx::is_pot(mDimsW) && cxx::is_pot(mDimsH);
}

int Texture2D_Data::GetMipmapsCount() const
{
    // not counting primary bitmap
    return (int) mMipmaps.size();
}

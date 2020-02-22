#include "pch.h"
#include "Texture2D_Data.h"
#include "stb_image_write.h"

Texture2D_Data::Texture2D_Data()
{
}

Texture2D_Data::~Texture2D_Data()
{
    ClearContent();
}

void Texture2D_Data::Create(eTextureFormat format, int sizex, int sizey, const unsigned char* sourceData)
{
    ClearContent();

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
    if (mPixelsFormat == eTextureFormat_Null)
        return false;

    int comp = GetTextureFormatBytesCount(mPixelsFormat);
    return stbi_write_png(filePath.c_str(), mDimsW, mDimsH, comp, mBitmap.data(), mDimsW * comp) > 0;
}

bool Texture2D_Data::SaveToFileMip(const std::string& filePath, int mipIndex)
{
    if (mPixelsFormat == eTextureFormat_Null)
        return false;

    int comp = GetTextureFormatBytesCount(mPixelsFormat);

    int mipSizex = GetTextureMipmapDimensions(mDimsW, mipIndex + 1);
    int mipSizey = GetTextureMipmapDimensions(mDimsH, mipIndex + 1);

    return stbi_write_png(filePath.c_str(), mipSizex, mipSizey, comp, mMipmaps[mipIndex].mBitmap.data(), mipSizex * comp) > 0;
}

void Texture2D_Data::CreateMipmap(const unsigned char* sourceData)
{
    if (mPixelsFormat == eTextureFormat_Null)
    {
        debug_assert(false);
        return;
    }

    int numMipmaps = (int) mMipmaps.size();
    int sizex = GetTextureMipmapDimensions(mDimsW, numMipmaps + 1);
    int sizey = GetTextureMipmapDimensions(mDimsH, numMipmaps + 1);
    const int dataLength = GetTextureFormatBytesCount(mPixelsFormat) * sizex * sizey;
    if (dataLength == 0)
    {
        debug_assert(false);
        return;
    }
    mMipmaps.emplace_back();
    if (sourceData)
    {
        mMipmaps[numMipmaps].mBitmap.assign(sourceData, sourceData + dataLength);
    }
    else
    {
        mMipmaps[numMipmaps].mBitmap.resize(dataLength);
    }
}

void Texture2D_Data::ClearMipmaps()
{
    mMipmaps.clear();
}

void Texture2D_Data::ClearContent()
{
    mBitmap.clear();
    mMipmaps.clear();
    mDimsH = 0;
    mDimsW = 0;
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
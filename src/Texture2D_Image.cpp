#include "pch.h"
#include "Texture2D_Image.h"
#include "3rd_party/stb_image_write.h"
#include "3rd_party/stb_image.h"

Texture2D_Image::Texture2D_Image()
{
    Clear();
}

Texture2D_Image::~Texture2D_Image()
{
    Clear();
}

bool Texture2D_Image::CreateImage(eTextureFormat format, const Point& dimensions, int mipmapsCount, bool transparent)
{
    if (format == eTextureFormat_Null)
    {
        debug_assert(false);
        return false;
    }

    if (dimensions.x < 1 || dimensions.y < 1)
    {
        debug_assert(false);
        return false;
    }

    if (mipmapsCount < 0)
    {
        debug_assert(false);
        mipmapsCount = 0;
    }

    Clear();

    mTextureDesc.mDimensions = dimensions;
    mTextureDesc.mImageDimensions = dimensions;
    mTextureDesc.mMipmapsCount = mipmapsCount;
    mTextureDesc.mTextureFormat = format;
    mTextureDesc.mTransparent = transparent;

    // count overall data size
    int dataLength = 0;
    for (int imipmap = 0; imipmap < mipmapsCount + 1; ++imipmap)
    {
        dataLength += GetImageDataSize(imipmap);
    }

    debug_assert(dataLength > 0);
    mImageData.resize(dataLength);
    return true;
}

bool Texture2D_Image::LoadFromFile(const std::string& filePath, eTextureFormat forceFormat)
{
    int dimx = 0;
    int dimy = 0;
    int num_channels = 0;
    int desired_channels = GetTextureFormatBytesCount(forceFormat);

    if (desired_channels == 0)
    {
        debug_assert(false);
        return false;
    }

    stbi_uc* image_data = stbi_load(filePath.c_str(), &dimx, &dimy, &num_channels, desired_channels);
    if (image_data == nullptr)
    {
        debug_assert(false);
        return false;
    }

    Clear();

    mTextureDesc.mDimensions.x = dimx;
    mTextureDesc.mDimensions.y = dimy;
    mTextureDesc.mImageDimensions.x = dimx;
    mTextureDesc.mImageDimensions.y = dimy;
    mTextureDesc.mTextureFormat = forceFormat;

    int dataLength = dimx * dimy * num_channels;
    debug_assert(dataLength > 0);
    mImageData.assign(image_data, image_data + dataLength);

    stbi_image_free(image_data);
    return true;
}

bool Texture2D_Image::DumpToFile(const std::string& filePath) const
{
    return DumpToFile(filePath, 0); // dump base image
}

bool Texture2D_Image::DumpToFile(const std::string& filePath, int mipmap) const
{
    if (!HasMipmap(mipmap))
    {
        debug_assert(false);
        return false;
    }
    const void* imageData = GetImageDataBuffer(mipmap);
    debug_assert(imageData);

    if (imageData == nullptr)
        return false;

    int dimx = GetTextureMipmapDims(mTextureDesc.mDimensions.x, mipmap);
    int dimy = GetTextureMipmapDims(mTextureDesc.mDimensions.y, mipmap);

    int comp = GetTextureFormatBytesCount(mTextureDesc.mTextureFormat);
    return stbi_write_png(filePath.c_str(), dimx, dimy, comp, imageData, dimx * comp) > 0;
}

void Texture2D_Image::ResizeToPowerOfTwo()
{
    if (IsNull())
    {
        debug_assert(false);
        return;
    }

    if (IsPowerOfTwo())
    {
        // nothing to do, pot already
        return;
    }

    // get nearest pot dimensions
    Point newTextureSize;
    newTextureSize.x = cxx::get_next_pot(mTextureDesc.mDimensions.x);
    newTextureSize.y = cxx::get_next_pot(mTextureDesc.mDimensions.y);
    
    mTextureDesc.mImageDimensions = mTextureDesc.mDimensions;
    mTextureDesc.mDimensions = newTextureSize;

    mTextureDesc.mMaxU = (mTextureDesc.mImageDimensions.x * 1.0f) / (newTextureSize.x * 1.0f);
    mTextureDesc.mMaxV = (mTextureDesc.mImageDimensions.y * 1.0f) / (newTextureSize.y * 1.0f);

    const int bpp = GetTextureFormatBytesCount(mTextureDesc.mTextureFormat);
    const int dataLength = bpp * newTextureSize.x * newTextureSize.y;
    const int srcRowLength = bpp * mTextureDesc.mImageDimensions.x;
    const int dstDowLength = bpp * newTextureSize.x;

    // copy old content
    ByteArray temporaryBuffer;
    temporaryBuffer.resize(dataLength);

    for (int currenty = 0; currenty < mTextureDesc.mImageDimensions.y; ++currenty)
    {
        int srcOffset = currenty * srcRowLength; 
        int dstOffset = currenty * dstDowLength;
        ::memcpy(temporaryBuffer.data() + dstOffset, mImageData.data() + srcOffset, srcRowLength);

        // put additional pixel to right
        srcOffset += srcRowLength - bpp;
        dstOffset += srcRowLength;
        ::memcpy(temporaryBuffer.data() + dstOffset, mImageData.data() + srcOffset, bpp);
    }
    // put additional line bottom
    int srcOffset = (mTextureDesc.mImageDimensions.y - 1) * dstDowLength;
    int dstOffset = mTextureDesc.mImageDimensions.y * dstDowLength;
    ::memcpy(temporaryBuffer.data() + dstOffset, temporaryBuffer.data() + srcOffset, srcRowLength + bpp);

    mImageData.swap(temporaryBuffer);
}

void Texture2D_Image::Clear()
{
    mTextureDesc = Texture2D_Desc();

    mImageData.clear();
}

void Texture2D_Image::Exchange(Texture2D_Image& sourceData)
{
    std::swap(mTextureDesc, sourceData.mTextureDesc);
    std::swap(mImageData, sourceData.mImageData);
}

bool Texture2D_Image::IsNull() const
{
    return mTextureDesc.mTextureFormat == eTextureFormat_Null || 
        mTextureDesc.mDimensions.x == 0 || mTextureDesc.mDimensions.y == 0 || mImageData.empty();
}

bool Texture2D_Image::IsPowerOfTwo() const
{
    bool wpot = cxx::get_next_pot(mTextureDesc.mDimensions.x) == mTextureDesc.mDimensions.x;
    bool hpot = cxx::get_next_pot(mTextureDesc.mDimensions.y) == mTextureDesc.mDimensions.y;

    return wpot && hpot;
}

bool Texture2D_Image::FillWithCheckerBoard(int mipmap)
{
    if (!HasMipmap(mipmap))
    {
        debug_assert(false);
        return false;
    }

    unsigned char* dataBuffer = GetImageDataBuffer(mipmap);
    int dimx = GetTextureMipmapDims(mTextureDesc.mDimensions.x, mipmap);
    int dimy = GetTextureMipmapDims(mTextureDesc.mDimensions.y, mipmap);

    if (mTextureDesc.mTextureFormat == eTextureFormat_R8 || mTextureDesc.mTextureFormat == eTextureFormat_R8UI)
    {
        for (int iy = 0; iy < dimy; ++iy)
            for (int ix = 0; ix < dimx; ++ix)
            {
                unsigned char color = 0xFF;
                if ((iy / 8) % 2 == (ix / 8) % 2)
                {
                    color = 0x00;
                }
                int offset = (iy * dimx) + ix;
                dataBuffer[offset] = color;
            }

        return true; 
    }

    if (mTextureDesc.mTextureFormat == eTextureFormat_RGB8)
    {
        for (int iy = 0; iy < dimy; ++iy)
            for (int ix = 0; ix < dimx; ++ix)
            {
                unsigned char color = 0xFF;
                if ((iy / 8) % 2 == (ix / 8) % 2)
                {
                    color = 0x00;
                }
                int offset = (iy * dimx * 3) + (ix * 3);
                dataBuffer[offset + 0] = color;
                dataBuffer[offset + 1] = 0x00;
                dataBuffer[offset + 2] = color;
            }

        return true;
    }

    if (mTextureDesc.mTextureFormat == eTextureFormat_RGBA8 || mTextureDesc.mTextureFormat == eTextureFormat_RGBA8UI)
    {
        for (int iy = 0; iy < dimy; ++iy)
            for (int ix = 0; ix < dimx; ++ix)
            {
                unsigned char color = 0xFF;
                if ((iy / 8) % 2 == (ix / 8) % 2)
                {
                    color = 0x00;
                }
                int offset = (iy * dimx * 4) + (ix * 4);
                dataBuffer[offset + 0] = color;
                dataBuffer[offset + 1] = 0x00;
                dataBuffer[offset + 2] = color;
                dataBuffer[offset + 3] = 0xFF;
            }

        return true;
    }
    return false;
}

bool Texture2D_Image::FillWithCheckerBoard()
{
    for (int imipmap = 0; imipmap < mTextureDesc.mMipmapsCount + 1; ++imipmap)
    {
        if (!FillWithCheckerBoard(imipmap))
        {
            debug_assert(false);
            break;
        }
    }
    return true;
}

bool Texture2D_Image::FillWithColor(Color32 color, int mipmap)
{
    if (!HasMipmap(mipmap))
    {
        debug_assert(false);
        return false;
    }

    unsigned char* dataBuffer = GetImageDataBuffer(mipmap);
    int dimx = GetTextureMipmapDims(mTextureDesc.mDimensions.x, mipmap);
    int dimy = GetTextureMipmapDims(mTextureDesc.mDimensions.y, mipmap);

    if (mTextureDesc.mTextureFormat == eTextureFormat_R8 || mTextureDesc.mTextureFormat == eTextureFormat_R8UI)
    {
        for (int iy = 0; iy < dimy; ++iy)
            for (int ix = 0; ix < dimx; ++ix)
            {
                int offset = (iy * dimx) + ix;
                dataBuffer[offset] = color.mR;
            }

        return true; 
    }

    if (mTextureDesc.mTextureFormat == eTextureFormat_RGB8)
    {
        for (int iy = 0; iy < dimy; ++iy)
            for (int ix = 0; ix < dimx; ++ix)
            {
                int offset = (iy * dimx * 3) + (ix * 3);
                dataBuffer[offset + 0] = color.mR;
                dataBuffer[offset + 1] = color.mG;
                dataBuffer[offset + 2] = color.mB;
            }

        return true;
    }

    if (mTextureDesc.mTextureFormat == eTextureFormat_RGBA8 || mTextureDesc.mTextureFormat == eTextureFormat_RGBA8UI)
    {
        for (int iy = 0; iy < dimy; ++iy)
            for (int ix = 0; ix < dimx; ++ix)
            {
                int offset = (iy * dimx * 4) + (ix * 4);
                dataBuffer[offset + 0] = color.mR;
                dataBuffer[offset + 1] = color.mG;
                dataBuffer[offset + 2] = color.mB;
                dataBuffer[offset + 3] = color.mA;
            }

        return true;
    }
    return false;
}

bool Texture2D_Image::FillWithColor(Color32 color)
{
    for (int imipmap = 0; imipmap < mTextureDesc.mMipmapsCount + 1; ++imipmap)
    {
        if (!FillWithColor(color, imipmap))
        {
            debug_assert(false);
            break;
        }
    }
    return true;
}

int Texture2D_Image::GetImageDataSize(int mipmap) const
{
    if (mTextureDesc.mTextureFormat == eTextureFormat_Null)
    {
        debug_assert(false);
        return 0;
    }

    int dataSize = GetTextureDataSize(mTextureDesc.mTextureFormat, mTextureDesc.mDimensions, mipmap);
    return dataSize;
}

bool Texture2D_Image::HasMipmap(int mipmap) const
{
    if (IsNull())
        return false;

    return mipmap >= 0 && mipmap <= mTextureDesc.mMipmapsCount;
}

unsigned char* Texture2D_Image::GetImageDataBuffer(int mipmap)
{
    if (HasMipmap(mipmap))
    {
        int dataOffset = 0;
        for (int icurr = 0; icurr < mipmap; ++icurr)
        {
            dataOffset += GetImageDataSize(icurr);
        }
        return mImageData.data() + dataOffset;
    }
    debug_assert(false);
    return nullptr;
}

const unsigned char* Texture2D_Image::GetImageDataBuffer(int mipmap) const
{
    if (HasMipmap(mipmap))
    {
        int dataOffset = 0;
        for (int icurr = 0; icurr < mipmap; ++icurr)
        {
            dataOffset += GetImageDataSize(icurr);
        }
        return mImageData.data() + dataOffset;
    }
    debug_assert(false);
    return nullptr;
}

unsigned char* Texture2D_Image::GetImageDataBuffer()
{
    return GetImageDataBuffer(0);
}

const unsigned char* Texture2D_Image::GetImageDataBuffer() const
{
    return GetImageDataBuffer(0);
}

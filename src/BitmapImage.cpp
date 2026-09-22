#include "stdafx.h"
#include "BitmapImage.h"

#include "stb_image.h"
#include "stb_image_write.h"

//////////////////////////////////////////////////////////////////////////

struct BitmapImage::PixelsAllocator
{};

//////////////////////////////////////////////////////////////////////////

BitmapImage::BitmapImage(const std::string& theFilePath)             
{
    LoadFromFile(theFilePath);
}

BitmapImage::BitmapImage(BitmapImage&& other) noexcept
{
    Swap(other);
}

BitmapImage::~BitmapImage()
{
    Clear();
}

void BitmapImage::SetMip(MipLevel& mipmap, const Point2D& dims, unsigned char* pixels, PixelsAllocator* bitsAllocator)
{
    FreeMip(mipmap);

    if (pixels == nullptr)
    {
        cxx_assert(bitsAllocator == nullptr);
        bitsAllocator = nullptr;

        const int dataLength = GetBytesPerPixel(mPixelFormat) * (dims.x * dims.y);

        // allocate memory
        pixels = new unsigned char[dataLength];
    }
    mipmap.mDims = dims;
    mipmap.mPixels = pixels;
    mipmap.mAllocator = bitsAllocator;
}

void BitmapImage::FreeMip(MipLevel& mipmap)
{
    if (mipmap.mPixels)
    {
        if (mipmap.mAllocator)
        {
            stbi_image_free(mipmap.mPixels);
        }
        else
        {
            // manual allocated
            delete [] mipmap.mPixels;
        }
    }
    mipmap.mDims = {};
    mipmap.mPixels = nullptr;
    mipmap.mAllocator = nullptr;
}

void BitmapImage::ReleaseMips()
{
    // do not release very first mip
    while (GetMipsCount() > 1)
    {
        FreeMip(mMipmaps.back());
        mMipmaps.pop_back();
    }
}

bool BitmapImage::AddMipLevel(const Point2D& dims, unsigned char** thePixels)
{
    if (!HasContent())
    {
        cxx_assert(false);
        return false;
    }

    cxx_assert((dims.x < mMipmaps.back().mDims.x) || (dims.y < mMipmaps.back().mDims.y));

    MipLevel& newMip = mMipmaps.emplace_back();
    SetMip(newMip, dims, nullptr);

    cxx_assert(newMip.mPixels);
    if (newMip.mPixels == nullptr)
    {
        mMipmaps.pop_back();
        return false;
    }

    if (thePixels)
    {
        *thePixels = newMip.mPixels;
    }
    return true;
}

bool BitmapImage::ResizeToPowerOfTwo()
{
    if (!HasContent())
        return false;

    for (int imipmap = 0; imipmap < GetMipsCount(); ++imipmap)
    {
        MipLevel& srcMip = mMipmaps[imipmap];
        Point2D dimsPOT 
        {
            cxx::get_next_pot(srcMip.mDims.x),
            cxx::get_next_pot(srcMip.mDims.y)
        };

        if (srcMip.mDims == dimsPOT)
            continue;

        MipLevel tempMip;
        SetMip(tempMip, dimsPOT, nullptr, nullptr);

        cxx_assert(tempMip.mPixels);
        if (tempMip.mPixels == nullptr)
            return false;

        // copy data
        const Point2D copyDims {
            std::min(srcMip.mDims.x, dimsPOT.x),
            std::min(srcMip.mDims.y, dimsPOT.y),
        };

        const unsigned int numBytesPerPixel = GetBytesPerPixel(mPixelFormat);
        const unsigned int copyBytesPerLine = copyDims.x * numBytesPerPixel;

        const int strideDst = dimsPOT.x * numBytesPerPixel;
        const int strideSrc = srcMip.mDims.x * numBytesPerPixel;

        unsigned char* ptrDst = tempMip.mPixels;
        unsigned char* ptrSrc = srcMip.mPixels;
        for (int iLine = 0; iLine < copyDims.y; ++iLine)
        {
            ::memcpy(ptrDst, ptrSrc, copyBytesPerLine);
            ptrDst += strideDst;
            ptrSrc += strideSrc;
        }

        // extent horz
        if (dimsPOT.x > srcMip.mDims.x)
        {
            unsigned char* ptrLineStart = tempMip.mPixels;
            for (int iLine = 0; iLine < copyDims.y; ++iLine)
            {
                ptrSrc = ptrLineStart + (copyBytesPerLine - numBytesPerPixel);
                ptrDst = ptrLineStart + copyBytesPerLine;
                for (int iPixel = copyDims.x; iPixel < dimsPOT.x; ++ iPixel)
                {
                    ::memcpy(ptrDst, ptrSrc, numBytesPerPixel);
                    ptrDst += numBytesPerPixel;
                }
                ptrLineStart += strideDst;
            }
        }
        // extent vert
        if (dimsPOT.y > srcMip.mDims.y)
        {
            ptrDst = tempMip.mPixels + strideDst * copyDims.y;
            ptrSrc = (ptrDst - strideDst); // last line
            
            for (int iLine = copyDims.y; iLine < dimsPOT.y; ++iLine)
            {
                ::memcpy(ptrDst, ptrSrc, strideDst);
                ptrDst += strideDst;
            }
        }

        FreeMip(srcMip);

        srcMip = tempMip;
    }
    return true;
}

bool BitmapImage::IsPOT() const 
{
    if (mMipmaps.empty()) 
        return false;

    for (const MipLevel& roller: mMipmaps)
    {
        cxx_assert(roller.mPixels);
        if (roller.mPixels == nullptr)
            continue;

        bool isPowerOfTwo = cxx::is_pot(roller.mDims.x) && cxx::is_pot(roller.mDims.y);
        if (!isPowerOfTwo)
            return false;
    }
    return true;
}

bool BitmapImage::Create(ePixelFormat thePixelFormat, const Point2D& dims, unsigned char* thePixels)
{
    Clear();

    if (thePixelFormat == ePixelFormat_Null)
        return false;

    cxx_assert(dims.x > 0);
    cxx_assert(dims.y > 0);

    if (dims.x < 1 || dims.y < 1)
        return false;

    mPixelFormat = thePixelFormat;

    MipLevel& baseMip = mMipmaps.emplace_back();
    SetMip(baseMip, dims, thePixels);

    cxx_assert(baseMip.mPixels);
    if (baseMip.mPixels == nullptr)
    {
        Clear();
        return false;
    }
    return true;
}

bool BitmapImage::Create(ePixelFormat thePixelFormat, const Point2D& dims, Color32 fillColor)
{
    if (Create(thePixelFormat, dims, nullptr))
    {
        ::memset(mMipmaps[0].mPixels, fillColor.mRGBA, dims.x * dims.y * GetBytesPerPixel(mPixelFormat));
        return true;
    }
    return false;
}

bool BitmapImage::CreateFrom(const BitmapImage& other, int mipmapLevel, int padding)
{
    Clear();

    if (!other.HasContent() || !other.HasMipLevel(mipmapLevel))
        return false;

    mPixelFormat = other.mPixelFormat;
    mHasAlphaHint = other.mHasAlphaHint;

    const unsigned char* srcImageData = other.GetMipPixels(mipmapLevel);
    cxx_assert(srcImageData);

    const Point2D srcImageDims = other.GetDimensions(mipmapLevel);
    if (padding < 1)
    {
        MipLevel& baseMip = mMipmaps.emplace_back();
        SetMip(baseMip, srcImageDims, nullptr);

        if (baseMip.mPixels)
        {
            const int imageDataSize = srcImageDims.x * srcImageDims.y * GetBytesPerPixel(mPixelFormat);
            ::memcpy(baseMip.mPixels, srcImageData, imageDataSize);
        }
        else
        {
            cxx_assert(false);
            Clear();
        }
    }
    else
    {
        const Point2D dstImageDims = srcImageDims + Point2D(padding * 2, padding * 2);

        MipLevel& baseMip = mMipmaps.emplace_back();
        SetMip(baseMip, dstImageDims, nullptr);

        if (baseMip.mPixels)
        {
            unsigned char* dstImageData = baseMip.mPixels;
            const int bytesPerPixel = GetBytesPerPixel(mPixelFormat);
            const int srcBytesPerLine = srcImageDims.x * bytesPerPixel;
            const int dstBytesPerLine = dstImageDims.x * bytesPerPixel;
            // center, left padding, right padding
            for (int iy = 0; iy < srcImageDims.y; ++iy)
            {
                auto* srcPtr = srcImageData + (iy * srcBytesPerLine);
                auto* dstPtrBase = dstImageData + ((iy + padding) * dstBytesPerLine);
                // center
                {
                    auto* dstPtr = dstPtrBase + (padding * bytesPerPixel);
                    ::memcpy(dstPtr, srcPtr, srcBytesPerLine);
                }
                // left padding
                srcPtr = srcImageData + (iy * srcBytesPerLine);
                for (int ix = 0; ix < padding; ++ix)
                {
                    auto* dstPtr = dstPtrBase + (ix * bytesPerPixel);
                    ::memcpy(dstPtr, srcPtr, bytesPerPixel);
                }
                // right padding
                srcPtr = srcImageData + ((iy * srcBytesPerLine) + (srcBytesPerLine - bytesPerPixel));
                for (int ix = 0; ix < padding; ++ix)
                {
                    auto* dstPtr = dstPtrBase + ((padding + srcImageDims.x + ix) * bytesPerPixel);
                    ::memcpy(dstPtr, srcPtr, bytesPerPixel);
                }
            }
            // top padding
            {
                auto* srcPtr = dstImageData + (padding * dstBytesPerLine);
                for (int iy = 0; iy < padding; ++iy)
                {
                    auto* dstPtr = dstImageData + (iy * dstBytesPerLine);
                    ::memcpy(dstPtr, srcPtr, dstBytesPerLine);
                }
            }
            // bottom padding
            {
                auto* srcPtr = dstImageData + ((padding + srcImageDims.y - 1) * dstBytesPerLine);
                for (int iy = 0; iy < padding; ++iy)
                {
                    auto* dstPtr = dstImageData + ((padding + srcImageDims.y + iy) * dstBytesPerLine);
                    ::memcpy(dstPtr, srcPtr, dstBytesPerLine);
                }
            }
        }
        else
        {
            cxx_assert(false);
            Clear();
        }
    }

    return HasContent();
}

bool BitmapImage::LoadFromFile(const std::string& theFilePath, ePixelFormat forceImageFormat)
{
    Clear();

    int imagew;
    int imageh;
    int imagecomponents;
    int forcecomponents = 0;

    if (forceImageFormat != ePixelFormat_Null)
    {
        forcecomponents = GetBytesPerPixel(forceImageFormat);
    }

    stbi_uc* pImageContent = stbi_load(theFilePath.c_str(), &imagew, &imageh, &imagecomponents, forcecomponents);
    if (!pImageContent)
    {
        cxx_assert(false);
        return false;
    }

    mPixelFormat = forceImageFormat;

    // pixels allocator is reserved
    static PixelsAllocator dummyAllocator;
    MipLevel& baseMip = mMipmaps.emplace_back();
    SetMip(baseMip, Point2D { imagew, imageh }, pImageContent, &dummyAllocator);

    cxx_assert(baseMip.mPixels);
    if (baseMip.mPixels == nullptr)
    {
        Clear();
        return false;
    }
    return true;
}

bool BitmapImage::LoadFromMemory(const unsigned char* dataBuffer, int dataLength, ePixelFormat forceImageFormat)
{
    Clear();

    int imagew;
    int imageh;
    int imagecomponents;
    int forcecomponents = 0;

    if (forceImageFormat != ePixelFormat_Null)
    {
        forcecomponents = GetBytesPerPixel(forceImageFormat);
    }

    stbi_uc* pImageContent = stbi_load_from_memory(dataBuffer, dataLength, &imagew, &imageh, &imagecomponents, forcecomponents);
    if (!pImageContent)
    {
        cxx_assert(false);
        return false;
    }

    mPixelFormat = forceImageFormat;

    // pixels allocator is reserved
    static PixelsAllocator dummyAllocator;
    MipLevel& baseMip = mMipmaps.emplace_back();
    SetMip(baseMip, Point2D { imagew, imageh }, pImageContent, &dummyAllocator);

    cxx_assert(baseMip.mPixels);
    if (baseMip.mPixels == nullptr)
    {
        Clear();
        return false;
    }
    return true;
}

bool BitmapImage::SaveToFile(const std::string& filePath) const
{
    return SaveToFile(filePath, 0);
}

bool BitmapImage::SaveToFile(const std::string& filePath, int mipMap) const
{
    if (!HasContent())
        return false;

    if (!HasMipLevel(mipMap))
    {
        cxx_assert(false);
        return false;
    }

    int componentsCount = GetBytesPerPixel(mPixelFormat);
    int strideBytes = mMipmaps[mipMap].mDims.x * componentsCount;
    return stbi_write_png(filePath.c_str(), 
        mMipmaps[mipMap].mDims.x, 
        mMipmaps[mipMap].mDims.y, componentsCount, mMipmaps[mipMap].mPixels, strideBytes) > 0;
}

void BitmapImage::SetHasAlphaHint(bool hasAlpha)
{
    mHasAlphaHint = hasAlpha;
}

void BitmapImage::Clear()
{
    mPixelFormat = ePixelFormat_Null;
    mHasAlphaHint = false;
    // free mips
    for (MipLevel& roller: mMipmaps)
    {
        FreeMip(roller);
    }
    mMipmaps.clear();
}

void BitmapImage::Swap(BitmapImage& other) noexcept
{
    std::swap(mHasAlphaHint, other.mHasAlphaHint);
    std::swap(mPixelFormat, other.mPixelFormat);
    std::swap(mMipmaps, other.mMipmaps);
}

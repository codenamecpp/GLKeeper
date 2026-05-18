#pragma once

#include "GraphicsDefs.h"

// Defines array of pixels in system memory
class BitmapImage
{
public:
    // Construct empty image
    BitmapImage();
    BitmapImage(const BitmapImage& other) = delete;
    BitmapImage(BitmapImage&& other);

    // Construct image from graphic file
    // @param theFilePath: Path to desired file
    // @param forceImageFormat: Convert loaded image format to specified
    BitmapImage(const std::string& theFilePath);
    ~BitmapImage();

    // Create image from graphic file
    // @param filePath: Path to desired file
    // @param forceImageFormat: Convert loaded image format to specified
    bool LoadFromFile(const std::string& theFilePath, ePixelFormat forceImageFormat = ePixelFormat_RGBA8);

    // Create image from memory buffer content
    // @param dataBuffer: Content data
    // @param dataLength: Content length
    // @param forceImageFormat: Convert loaded image format to specified
    bool LoadFromMemory(const unsigned char* dataBuffer, int dataLength, ePixelFormat forceImageFormat = ePixelFormat_RGBA8);

    // Create image from memory
    bool Create(ePixelFormat thePixelFormat, const Point2D& dims, unsigned char* thePixels);
    bool Create(ePixelFormat thePixelFormat, const Point2D& dims, Color32 fillColor);

    // Save image content to external file
    bool SaveToFile(const std::string& filePath) const;
    bool SaveToFile(const std::string& filePath, int mipMap) const;

    void SetHasAlphaHint(bool hasAlpha);
    bool GetHasAlphaHint() const { return mHasAlphaHint; }

    // Destroy image data with mips
    void Clear();

    void Swap(BitmapImage& other);

    // Test whether image is not empty
    inline bool HasContent() const 
    { 
        return !mMipmaps.empty(); 
    }

    // Get image pixel format
    // @return Pixel format or IMAGE_PIXEL_FORMAT_NULL if image is empty
    inline ePixelFormat GetPixelFormat() const { return mPixelFormat; }

    // Get number of mip levels specified, including primary image
    inline int GetMipsCount() const 
    { 
        return static_cast<int>(mMipmaps.size()); 
    }

    // Get pixels of specified mip level
    inline unsigned char* GetMipPixels(int theMip) 
    {
        cxx_assert(theMip < GetMipsCount());
        return mMipmaps[theMip].mPixels;
    }

    // Get pixels of specified mip level for reading
    inline const unsigned char* GetMipPixels(int theMip) const 
    {
        cxx_assert(theMip < GetMipsCount());
        return mMipmaps[theMip].mPixels; 
    }

    // Test whether mip level is specified
    inline bool HasMipLevel(int theMip) const 
    { 
        return theMip < GetMipsCount(); 
    }

    // Create new mip level
    bool AddMipLevel(const Point2D& dims, unsigned char** thePixels);

    // Resize base image and all mipmaps to nearest pot dimensions
    bool ResizeToPowerOfTwo();

    // Delete all mip levels except for mip 0
    void ReleaseMips();

    // Get image dimensions
    // @param theDimX: Output width
    // @param theDimY: Output height
    inline void GetDimensions(int& theDimX, int& theDimY) const 
    {
        theDimX = mMipmaps[0].mDims.x;
        theDimY = mMipmaps[0].mDims.y;
    }

    inline Point2D GetDimensions() const { return mMipmaps[0].mDims; }
    inline Point2D GetDimensions(int mipLevel) const 
    { 
        cxx_assert(mipLevel < GetMipsCount());
        return mMipmaps[mipLevel].mDims; 
    }

    // Test whether image dimensions is power of two
    bool IsPOT() const;

    BitmapImage& operator = (const BitmapImage& other) = delete;
    BitmapImage& operator = (BitmapImage&& other)
    {
        Swap(other);
        return *this;
    }

private:
    struct PixelsAllocator;

    //////////////////////////////////////////////////////////////////////////
    struct MipLevel
    {
    public:
        PixelsAllocator* mAllocator = nullptr;
        Point2D mDims {};
        unsigned char* mPixels = nullptr;
    };
    //////////////////////////////////////////////////////////////////////////

    // internal procs
    void SetMip(MipLevel& mipmap, const Point2D& dims, unsigned char* pixels, PixelsAllocator* bitsAllocator = nullptr);
    void FreeMip(MipLevel& mipmap);

    //////////////////////////////////////////////////////////////////////////

    ePixelFormat mPixelFormat;

    bool mHasAlphaHint = false;

    std::vector<MipLevel> mMipmaps; // 0 mip level is primary
};

//////////////////////////////////////////////////////////////////////////
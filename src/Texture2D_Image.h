#pragma once

// contains texture2d image data and properties
class Texture2D_Image
{
public:
    Texture2D_Desc mTextureDesc;

public:
    Texture2D_Image();
    ~Texture2D_Image();

    // allocate data buffer for base image and mipmaps
    // will destroy old content
    // @param format: Pixels format
    // @param dimensions: Base image dimensions
    // @param mipmapsCount: Number of additional mipmap levels that will be allocated
    // @param transparent: Image has transparent pixels
    bool CreateImage(eTextureFormat format, const Point& dimensions, int mipmapsCount, bool transparent);
    
    // clear image data
    void Clear();

    // swap image data
    // @param sourceData: Source image data to swap with
    void Exchange(Texture2D_Image& sourceData);

    // save image content to external file
    // @param filePath: File path
    // @param mipmap: Mipmap level, 0 is for base image
    bool DumpToFile(const std::string& filePath) const;
    bool DumpToFile(const std::string& filePath, int mipmap) const;

    // create checker board pattern, base image must be allocated
    // @param mipmap: Mipmap level, 0 is for base image
    // @returns false if pixels format not supported
    bool FillWithCheckerBoard(int mipmap);
    bool FillWithCheckerBoard();

    // fill canvas with color, base image must be allocated
    // @param mipmap: Mipmap level, 0 is for base image
    // @returns false if pixels format not supported
    bool FillWithColor(Color32 color, int mipmap);
    bool FillWithColor(Color32 color);

    // resize base image to nearest pot dimensions, discards any mipmaps
    // source image size will be stored in mImageSize
    void ResizeToPowerOfTwo();

    // test whether texture data is null
    bool IsNull() const;

    // test whether texture bitmap has power of two dimensions
    bool IsPowerOfTwo() const;

    // test whether image has additional mipmap levels
    inline bool HasMipmaps() const
    {
        return mTextureDesc.mMipmapsCount > 0;
    }

    // test whether image has specific mipmap level
    inline bool HasMipmap(int mipmap) const;

    // get image data buffer
    // @param mipmap: Mipmap level, 0 is for base image
    // @returns num bytes within mImageData array
    unsigned char* GetImageDataBuffer(int mipmap);
    unsigned char* GetImageDataBuffer();

    const unsigned char* GetImageDataBuffer(int mipmap) const;
    const unsigned char* GetImageDataBuffer() const;

    // get image data length in bytes
    // @param mipmap: Mipmap level, 0 is for base image
    int GetImageDataSize(int mipmap) const;

private:
    // image data array contains base image along with all mipmaps
    ByteArray mImageData;
};
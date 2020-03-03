#pragma once

#include "GraphicsDefs.h"

// contains texture2d bitmap data and properties
class Texture2D_Data
{
public:
    eTextureFormat mPixelsFormat = eTextureFormat_Null;

    // data of single mipmap level
    struct MipmapData
    {
    public:
        ByteArray mBitmap;
        Size2D mSize;
    };
    std::vector<MipmapData> mMipmaps;

    // contains pixels data for primary texture bitmap
    ByteArray mBitmap;

    // texture dimensions
    Size2D mSize;
    Size2D mImageSize; // NPOT size of texture image

    float mMaxU;
    float mMaxV;

    bool mTransparent; // texture has transparent pixels

public:
    Texture2D_Data();
    ~Texture2D_Data();
    
    // allocates buffer for bitmap of specified format and dimensions
    // @param format: Pixels format
    // @param dimensions: Texture dimensions
    // @param transparent: Texture has transparent pixels
    // @param sourceData: Source pixels data, optional
    void Setup(eTextureFormat format, const Size2D& dimensions, bool transparent, const unsigned char* sourceData);
    void Clear();

    // allocate memory for mipmaps, bitmap params must be specified
    // @param mipmapsCount: New mipmaps count not counting primary bitmap
    void SetupMipmaps(int mipmapsCount);
    void ClearMipmaps();

    // create checker board pattern, bitmap must be allocated
    // does not affects on mipmaps
    // @returns false if pixels format not supported
    bool FillWithCheckerBoard();
    bool FillWithCheckerBoard(const Rect2D& fillArea);

    // fill canvas with color, bitmap must be allocated
    // does not affects on mipmaps
    // @returns false if pixels format not supported
    bool FillWithColor(Color32 color);
    bool FillWithColor(Color32 color, const Rect2D& fillArea);

    // resize texture image to nearest pot dimensions
    // actual size of image will be stored in mImageSize
    // @param freeCurrentBitmap: Don't keep old image content
    void ResizeToPOT(bool freeCurrent);

    // save bitmap content to external file
    // @param filePath: File path
    bool DumpToFile(const std::string& filePath);

    // get number of additional mipmaps not counting primary bitmap
    int GetMipmapsCount() const;

    // test whether texture has additional mipmap levels
    bool HasMipmaps() const;

    // swap texture data
    // @param sourceData: Source texture data container to swap with
    void SwapContent(Texture2D_Data& sourceData);

    // test whether texture data is null
    bool IsNull() const;
    bool NonNull() const;

    // test whether texture bitmap has power of two dimensions
    bool IsPOT() const;
    bool NonPOT() const;
};
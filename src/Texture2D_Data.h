#pragma once

#include "GraphicsDefs.h"

// contains texture2d bitmap data and properties
class Texture2D_Data
{
public:

    // data for single mipmap level
    struct MipmapData
    {
    public:
        int mDimsW = 0;
        int mDimsH = 0;

        ByteArray mBitmap;
    };
    std::vector<MipmapData> mMipmaps;

    // contains pixels data for primary texture bitmap
    ByteArray mBitmap;

    // texture dimensions
    int mDimsW = 0;
    int mDimsH = 0;

    bool mTransparent = false;

    eTextureFormat mPixelsFormat = eTextureFormat_Null;

public:
    Texture2D_Data();
    ~Texture2D_Data();
    
    // allocates buffer for bitmap of specified format and dimensions
    // @param format: Pixels format
    // @param sizex, sizey: Texture dimensions
    // @param sourceData: Source pixels data, optional
    void Setup(eTextureFormat format, int sizex, int sizey, bool transparent, const unsigned char* sourceData);
    void Clear();

    // allocate memory for mipmaps, bitmap params must be specified
    // @param mipmapsCount: New mipmaps count not counting primary bitmap
    void SetupMipmaps(int mipmapsCount);
    void ClearMipmaps();

    // test whether texture has additional mipmap levels
    inline bool HasMipmaps() const
    {
        return GetMipmapsCount() > 0;
    }

    // get number of additional mipmaps not counting primary bitmap
    int GetMipmapsCount() const;

    // save bitmap content to external file
    // @param filePath: File path
    // @param mipmapIndex: Mipmap level
    bool SaveToFile(const std::string& filePath);
    bool SaveToFileMip(const std::string& filePath, int mipmapIndex);

    // swap data
    // @param sourceData: Source texture data container to swap with
    void SwapContent(Texture2D_Data& sourceData);

    // test whether texture data is null
    bool IsNull() const;

    // test whether texture bitmap has power of two dimensions
    bool IsPOT() const;
};
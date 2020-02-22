#pragma once

#include "GraphicsDefs.h"

// contains texture2d bitmap data and properties
class Texture2D_Data
{
public:
    
    // contains data for single mipmap level
    struct MipmapData
    {
        ByteArray mBitmap;
    };
    std::vector<MipmapData> mMipmaps;

    // texture dimensions
    int mDimsW = 0;
    int mDimsH = 0;

    eTextureFormat mPixelsFormat = eTextureFormat_Null;

    // contains pixels data for texture
    ByteArray mBitmap;

public:
    Texture2D_Data();
    ~Texture2D_Data();
    
    // allocates buffer for bitmap of specified format and dimensions
    // @param format: Pixels format
    // @param sizex, sizey: Texture dimensions
    // @param sourceData: Source pixels data, optional
    void Create(eTextureFormat format, int sizex, int sizey, const unsigned char* sourceData);

    // allocate mipmap data
    // @param sourceData: Source pixels data, optional
    void CreateMipmap(const unsigned char* sourceData);

    // save bitmap content to external file
    // @param filePath: File path
    // @param mipIndex: Mipmap level
    bool SaveToFile(const std::string& filePath);
    bool SaveToFileMip(const std::string& filePath, int mipIndex);

    // free texture data
    void ClearContent();
    void ClearMipmaps();

    // swap data
    // @param sourceData: Source texture data container to swap with
    void SwapContent(Texture2D_Data& sourceData);

    // test whether texture data is null
    bool IsNull() const;

    // test whether texture bitmap has power of two dimensions
    bool IsPOT() const;
};
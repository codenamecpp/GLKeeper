#pragma once

//////////////////////////////////////////////////////////////////////////

#include "FileSystem.h"
#include "BitmapImage.h"

//////////////////////////////////////////////////////////////////////////

enum { MAX_ENGINE_TEXTURE_MIPS = 16 };
struct DK2EngineTextureMip
{
public:
    int mEntryIndex = -1;
    int mDimX = 0;
    int mDimY = 0;
};

struct DK2EngineTextureDesc
{
public:
    DK2EngineTextureMip mMips[MAX_ENGINE_TEXTURE_MIPS];
    int mMipsCount = 0;
    bool mHasAlpha = false;
};

//////////////////////////////////////////////////////////////////////////

using DK2EngineTextureID = unsigned int;

//////////////////////////////////////////////////////////////////////////

class DK2EngineTexturesCache: public cxx::noncopyable
{
public:
    DK2EngineTexturesCache();
    ~DK2EngineTexturesCache();

    // Scan dungeon keeper 2 textures cache
    // @param theTexturesCachePath: Path to dungeon keeper 2 textures cache folder
    bool ScanDungeonKeeperTexturesCache(const std::string& theTexturesCachePath);

    // Free allocated resources
    void Shutdown();

    // Find texture within engine cache by name
    // @param theTextureName: Texture name without extension
    // @param theTextureID: Output texture ID
    bool FindTextureByName(const std::string& theTextureName, DK2EngineTextureID& theTextureID) const;

    // Extract information about engine texture
    // @param theTextuteID: Texture ID
    // @param outputInfo: Output information
    bool ExtractTextureInfo(DK2EngineTextureID theTextuteID, DK2EngineTextureDesc& outputInfo) const;

    bool GetTextureNameByID(DK2EngineTextureID theTextureID, std::string& textureName) const;

    // Extract texture with all mipmaps from engine textures cache
    // @param theTextureID: Unique texture ID
    // @param outputBitmap: Output pixels
    bool ExtractTexture(DK2EngineTextureID theTextureID, BitmapImage& outputBitmap) const;

    // Decode 444 loading screen texture to RGBA
    // Note that result image will have non power of two dimensions, it should be handled manually
    bool Decompress444(const ByteArray& textureData, BitmapImage& outputImage);

private:
    //////////////////////////////////////////////////////////////////////////
    using IndicesMap = std::unordered_map<std::string, DK2EngineTextureID, cxx::icase_string_hashfunc, cxx::icase_string_eq>;
    //////////////////////////////////////////////////////////////////////////
    IndicesMap mIndices;
    std::vector<DK2EngineTextureDesc> mEntries;
};
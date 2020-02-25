#pragma once

#include "CommonTypes.h"
#include "GraphicsDefs.h"

// dungeon keeper engine textures provider
class EngineTexturesProvider: public cxx::noncopyable
{
public:

    EngineTexturesProvider();
    ~EngineTexturesProvider();

    // setup engine textures cache, returns false on error
    bool Initialize();
    void Deinit();

    // test whether specific texture exists within engine textures cache
    // @param textureName: Entry name
    bool ContainsTexture(const std::string& textureName) const;

    // extract texture with all mipmaps from engine textures cache
    // @param textureName: Entry name
    // @param textureData: Output texture data
    bool ExtractTexture(const std::string& textureName, Texture2D_Data& textureData) const;

    // extract all textures with their mipmaps to specified directory
    // @param outputDirectory: Output directory path
    void DumpTextures(const std::string& outputDirectory) const;

private:
    bool ParseDirContent(const std::string& dirFilepath, const std::string& datFilepath);

    // decompress texture data
    bool ExtractTexturePixels(int entryIndex, unsigned char* outputBuffer) const;

private:

    enum 
    {
        TEXTURE_MIP_LEVEL_HIGH,         // 128x128
        TEXTURE_MIP_LEVEL_MEDIUM,       // 64x64
        TEXTURE_MIP_LEVEL_LOW,          // 32x32
        TEXTURE_MIP_LEVEL_VERY_LOW,     // 16x16
        TEXTURE_MIP_LEVEL_COUNT
    };
    
    // description for single texture stored within cache
    struct TextureEntryStruct
    {
    public:
        int mDataLength;
        int mDataStartLocation;
        int mSizeX;
        int mSizeY;
        short mSizeX2;
        short mSizeY2;
        bool mHasAlpha;
    };

    struct TextureEntryIndex
    {
    public:
        int mMipsCount = 0;
        int mMipIndices[TEXTURE_MIP_LEVEL_COUNT] = {-1, -1, -1, -1};
    };

    using TextureIndicesMap = std::map<std::string, TextureEntryIndex, cxx::icase_less>;
    TextureIndicesMap mIndicesMap;

    std::vector<TextureEntryStruct> mEntiesArray; // all textures and mipmaps entries

    FILE* mFileStream = nullptr;
};

extern EngineTexturesProvider gEngineTexturesProvider;
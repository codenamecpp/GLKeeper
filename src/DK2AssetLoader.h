#pragma once

#include "DK2EngineTextures.h"
#include "DK2WADArchive.h"
#include "DK2Font.h"
#include "DK2KMFModel.h"

//////////////////////////////////////////////////////////////////////////

class DK2AssetLoader: public cxx::noncopyable
{
public:
    // Scan resource paths and open arhive wads
    bool Initialize();
    void Shutdown();

    // Extract texture resource data
    bool LoadImageData(const std::string& theTextureName, BitmapImage& outputBitmap);

    // Extract mesh resource data
    bool LoadKMFModelData(const std::string& theMeshName, DK2KMFModel& outputModel);

    // Extract font resource data
    bool LoadBF4FontData(const std::string& theFontName, DK2FontDesc& outputFontData);

    bool GetArchiveNames(std::vector<std::string>& namesList) const;
    bool GetArchiveEntryNames(const std::string& archiveName, std::vector<std::string>& entryNames) const;

    // Direct access to archive
    DK2WADArchive* GetArchiveByName(const std::string& archiveName) const;

private:
    bool LoadWADs();
    void FreeWADs();
    bool LoadFromWADs(const std::string& theResourceName, ByteArray& theOutputData);

private:
    DK2EngineTexturesCache mEngineTexturesCache;
    std::vector<DK2WADArchive*> mArchives;
    ByteArray mReadBuffer;
};

//////////////////////////////////////////////////////////////////////////

extern DK2AssetLoader gDK2AssetLoader;

//////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////////////////////////////////////////////

#include "AudioDefs.h"
#include "DK2SfxMapFile.h"
#include "DK2SfxBankFile.h"
#include "DK2SoundArchive.h"

//////////////////////////////////////////////////////////////////////////

namespace SoLoud
{
    class Soloud;
    class WavStream;
}

//////////////////////////////////////////////////////////////////////////

class AudioManager final: public cxx::noncopyable
{
private:
    //////////////////////////////////////////////////////////////////////////
    using SfxFilesPair = std::pair<DK2SfxMapFile, DK2SfxBankFile>;
    //////////////////////////////////////////////////////////////////////////
    struct SfxEndpoint
    {
        DK2SoundArchive* mSoundArchive {};
        // sound index within sound archive, 0 based
        unsigned int mEntryIndex {};
    };
    //////////////////////////////////////////////////////////////////////////
public:
    AudioManager();
    ~AudioManager();

    bool Initialize();
    void Shutdown();

    bool IsAudioOnline() const;

    bool PlayOneShot(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIndex);
    bool PlayOneShot(const std::string& categoryName, snd_group_id groupId);

private:
    bool ScanMapFiles();

    bool ResolveSound(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIdx, SfxEndpoint& endpoint);
    bool ResolveSound(const std::string& categoryName, snd_group_id groupId, SfxEndpoint& endpoint);

    DK2SoundArchive* OpenSoundArchive(const std::string& archiveName);

private:
    //////////////////////////////////////////////////////////////////////////
    std::string mSoundSfxDirectoryPath;
    // category name is in upper case
    std::unordered_map<std::string, SfxFilesPair> mCategoriesMap;
    std::unordered_map<std::string, DK2SoundArchive> mSoundArhivesMap;

    std::unique_ptr<SoLoud::Soloud> mSoundEngine;
    std::unordered_map<std::string, std::unique_ptr<SoLoud::WavStream>> mClipsCache;

    std::vector<uint8_t> mDataBuffer;
};

//////////////////////////////////////////////////////////////////////////

extern AudioManager gAudio;

//////////////////////////////////////////////////////////////////////////
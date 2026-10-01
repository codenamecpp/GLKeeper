#pragma once

//////////////////////////////////////////////////////////////////////////

#include "AudioDefs.h"
#include "DK2SfxMapFile.h"
#include "DK2SfxBankFile.h"
#include "DK2SoundArchive.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

namespace SoLoud
{
    class Soloud;
    class Bus;
    class Queue;
    class AudioSource;
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
    void UpdateFrame(float deltaTime);
    void SetMasterVolume(float volume);
    void SetVoiceVolume(float volume);
    void SetMusicVolume(float volume);
    void SetSfxVolume(float volume);

    bool IsAudioOnline() const;

    bool PlayOneShot(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIndex);
    bool PlayOneShot(const std::string& categoryName, snd_group_id groupId);

    bool PlayAmbience(const std::string& categoryName, snd_group_id groupId);
    bool StopAmbience();
    bool IsAmbiencePlaying() const;

private:
    bool ScanMapFiles();
    bool ResolveSound(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIdx, SfxEndpoint& endpoint);
    bool ResolveSound(const std::string& categoryName, snd_group_id groupId, SfxEndpoint& endpoint);

    void UpdateAmbience(bool isInitial);

    SoLoud::AudioSource* LoadSound(const SfxEndpoint& endpoint, bool queueableAudio);

    DK2SoundArchive* OpenSoundArchive(const std::string& archiveName);

    SfxFilesPair& GetSfxFilesPair(const std::string& categoryName);

private:
    std::string mSoundSfxDirectoryPath;

    // category name is in upper case
    std::unordered_map<std::string, SfxFilesPair> mCategoriesMap;
    std::unordered_map<std::string, DK2SoundArchive> mSoundArhivesMap;

    std::unique_ptr<SoLoud::Soloud> mAudioEngine;
    std::unordered_map<std::string, std::unique_ptr<SoLoud::AudioSource>> mClipsCache;

    std::unique_ptr<SoLoud::Bus> mSoundBusGui;
    std::unique_ptr<SoLoud::Bus> mSoundBusMusic;
    std::unique_ptr<SoLoud::Bus> mSoundBusSpeech;
    std::unique_ptr<SoLoud::Bus> mSoundBusWorld;

    std::unique_ptr<SoLoud::Queue> mSoundQueueAmbience;

    std::vector<uint8_t> mDataBuffer;

    // ambience playback state
    std::vector<SoLoud::AudioSource*> mAmbienceSequence;
    size_t mAmbienceSequencePos {}; // points to next clip in mAmbienceSequence
    SimpleTimer mAmbienceUpdateTimer {};
};

//////////////////////////////////////////////////////////////////////////

extern AudioManager gAudio;

//////////////////////////////////////////////////////////////////////////
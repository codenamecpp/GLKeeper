#include "stdafx.h"
#include "AudioManager.h"
// SoLoud
#include "SoLoud/include/soloud.h"
#include "SoLoud/include/soloud_wavstream.h"
#include "SoLoud/include/soloud_wav.h"

//////////////////////////////////////////////////////////////////////////

AudioManager gAudio;

//////////////////////////////////////////////////////////////////////////

inline bool _SoLoudCheckResult_(SoLoud::result code, const char* functionName)
{
    bool isSuccess = (code == SoLoud::SO_NO_ERROR);
    if (!isSuccess)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Audio engine error in %s: '%s'", functionName, SoLoud::Soloud::getErrorString(code));
        cxx_assert(false); 
    }
    return isSuccess;
}

#define SoLoudCheckResult(code) _SoLoudCheckResult_(code, __FUNCTION__)

//////////////////////////////////////////////////////////////////////////

AudioManager::AudioManager()
{

}

AudioManager::~AudioManager()
{
    cxx_assert(!mAudioEngine);
}

bool AudioManager::Initialize()
{
    if (!gFiles.PathToDirectory("Data/Sound/Sfx", mSoundSfxDirectoryPath))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot locate sounds");
        return true;
    }

    gConsole.LogMessage(eLogLevel_Info, "Sounds location: '%s'", mSoundSfxDirectoryPath.c_str());

    ScanMapFiles();

    gConsole.LogMessage(eLogLevel_Info, "Initialize SoLoud audio engine...");

    mAudioEngine = std::make_unique<SoLoud::Soloud>();
    if (!SoLoudCheckResult(mAudioEngine->init()))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Failed to initialize audio engine");

        mAudioEngine.reset();
        return false;
    }

    gConsole.LogMessage(eLogLevel_Info, "Audio engine backend string: %s", mAudioEngine->getBackendString());

    mSoundBusGui = std::make_unique<SoLoud::Bus>();
    mAudioEngine->play(*mSoundBusGui);

    mSoundQueueAmbience = std::make_unique<SoLoud::Queue>();

    return true;
}

void AudioManager::Shutdown()
{
    StopAmbience();

    for (auto& roller: mClipsCache)
    {
        roller.second->stop();
    }

    mClipsCache.clear();

    if (mSoundQueueAmbience)
    {
        mSoundQueueAmbience->stop_queue();
        mSoundQueueAmbience->stop();
        mSoundQueueAmbience.reset();
    }

    if (mSoundBusGui)
    {
        mSoundBusGui->stop();
        mSoundBusGui.reset();
    }

    if (mSoundBusMusic)
    {
        mSoundBusMusic->stop();
        mSoundBusMusic.reset();
    }

    if (mSoundBusSpeech)
    {
        mSoundBusSpeech->stop();
        mSoundBusSpeech.reset();
    }

    if (mSoundBusWorld)
    {
        mSoundBusWorld->stop();
        mSoundBusWorld.reset();
    }

    if (mAudioEngine)
    {
        mAudioEngine->deinit();
        mAudioEngine.reset();
    }

    mCategoriesMap.clear();
    mSoundArhivesMap.clear();
}

void AudioManager::SetMasterVolume(float volume)
{
    if (mAudioEngine)
    {
        mAudioEngine->setGlobalVolume(volume);
    }
}

void AudioManager::SetVoiceVolume(float volume)
{
    // todo
}

void AudioManager::SetMusicVolume(float volume)
{
    // todo
}

void AudioManager::SetSfxVolume(float volume)
{
    if (mSoundBusGui)
    {
        mSoundBusGui->setVolume(volume);
    }

    if (mSoundQueueAmbience)
    {
        mSoundQueueAmbience->setVolume(volume);
    }
}

bool AudioManager::ScanMapFiles()
{
    mCategoriesMap.clear();

    namespace fs = std::filesystem;

    const fs::path soundsRoot = fs::path {mSoundSfxDirectoryPath};
    if (!fs::exists(soundsRoot) || !fs::is_directory(soundsRoot))
    {
        cxx_assert(false);
        return false;
    }
    
    const std::string mapFileExt = ".map";
    const std::string mapFileNameSuffixSfx = "sfx";
    const std::string mapFileNameSuffixBank = "bank";

    for (const auto& rootEntry : fs::directory_iterator(soundsRoot))
    {
        if (!fs::is_directory(rootEntry))
            continue;

        const fs::path& subDir = rootEntry.path();
        for (const auto& subEntry : fs::directory_iterator(subDir))
        {
            if (!subEntry.is_regular_file())
                continue;

            fs::path filePath = subEntry.path();
            if (!filePath.has_filename() || 
                !filePath.has_extension() || !cxx::ends_with_icase(filePath.extension().string(), mapFileExt))
            {
                continue;
            }
            
            const std::string fileNameWithoutExt = filePath.stem().string();

            std::string categoryName;

            enum eMapFileContent { eMapFileContent_Unknown, eMapFileContent_Bank, eMapFileContent_Sfx };
            eMapFileContent fileContent = eMapFileContent_Unknown;

            if ((fileContent == eMapFileContent_Unknown) && cxx::ends_with_icase(fileNameWithoutExt, mapFileNameSuffixBank))
            {
                categoryName = fileNameWithoutExt.substr(0, fileNameWithoutExt.length() - mapFileNameSuffixBank.length());
                fileContent = eMapFileContent_Bank;
            }

            if ((fileContent == eMapFileContent_Unknown) && cxx::ends_with_icase(fileNameWithoutExt, mapFileNameSuffixSfx))
            {
                categoryName = fileNameWithoutExt.substr(0, fileNameWithoutExt.length() - mapFileNameSuffixSfx.length());
                fileContent = eMapFileContent_Sfx;
            }

            if (fileContent == eMapFileContent_Unknown)
                continue;

            cxx_assert(!categoryName.empty());
            if (categoryName.empty())
                continue;

            cxx::to_upper(categoryName);
    
            SfxFilesPair& mapFilesPair = mCategoriesMap[categoryName];

            if (fileContent == eMapFileContent_Bank)
            {
                if (mapFilesPair.second.IsOpened())
                {
                    cxx_assert(false);
                    continue;
                }
                if (!mapFilesPair.second.OpenFile(filePath.string()))
                {
                    cxx_assert(false);
                    gConsole.LogMessage(eLogLevel_Warning, "Cannot load sfx bank file for category '%s'", categoryName.c_str());
                }
                continue;
            }

            if (fileContent == eMapFileContent_Sfx)
            {
                if (mapFilesPair.first.IsOpened())
                {
                    cxx_assert(false);
                    continue;
                }
                if (!mapFilesPair.first.OpenFile(filePath.string()))
                {
                    cxx_assert(false);
                    gConsole.LogMessage(eLogLevel_Warning, "Cannot load sfx map file for category '%s'", categoryName.c_str());
                }
                continue;
            }

            cxx_assert(false);
        }
    }

    return !mCategoriesMap.empty();
}

bool AudioManager::PlayOneShot(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIndex)
{
    if (!IsAudioOnline())
        return false;

    SfxEndpoint soundEndpoint;
    if (!ResolveSound(categoryName, groupId, clipIndex, soundEndpoint))
        return false;

    if (SoLoud::AudioSource* audioSource = LoadSound(soundEndpoint, false))
    {
        mSoundBusGui->play(*audioSource);
        return true;
    }

    return false;
}

bool AudioManager::PlayOneShot(const std::string& categoryName, snd_group_id groupId)
{
    return PlayOneShot(categoryName, groupId, -1);
}

DK2SoundArchive* AudioManager::OpenSoundArchive(const std::string& archiveName)
{
    if (mSoundArhivesMap.find(archiveName) == mSoundArhivesMap.end())
    {
        namespace fs = std::filesystem;

        const fs::path soundsRoot = fs::path {mSoundSfxDirectoryPath};
        const fs::path archiveHWPath = soundsRoot / (archiveName + "HW.sdt");
        const fs::path archiveHDPath = soundsRoot / (archiveName + "HD.sdt");
        const std::string archiveFilePath = fs::exists(archiveHWPath) ? 
            archiveHWPath.string() :
            archiveHDPath.string();

        if (!mSoundArhivesMap[archiveName].OpenArchive(archiveFilePath))
        {
            cxx_assert(false);

            gConsole.LogMessage(eLogLevel_Warning, "Cannot open sound archive '%s'", archiveName.c_str());
            return nullptr;
        }
    }
    DK2SoundArchive& soundArchive = mSoundArhivesMap[archiveName];
    return soundArchive.IsArchiveOpened() ? &soundArchive : nullptr;
}

bool AudioManager::IsAudioOnline() const
{
    return mAudioEngine.get() != nullptr;
}

bool AudioManager::ResolveSound(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIdx, SfxEndpoint& endpoint)
{
    endpoint = {};

    const SfxFilesPair& mapFilesPair = GetSfxFilesPair(categoryName);

    // select random clip
    if (clipIdx == -1)
    {
        unsigned int soundEntriesCount {};
        if (!mapFilesPair.first.GetSoundEntriesCount(groupId, 0, soundEntriesCount) || (soundEntriesCount == 0))
            return false;

        clipIdx = Random::GenerateUint(0, soundEntriesCount - 1);
    }

    DK2SfxMapFile::SfxSoundEntry mapSoundEntry;
    if (!mapFilesPair.first.GetSoundEntry(groupId, 0, clipIdx, mapSoundEntry) ||
        (mapSoundEntry.mIndex == 0) || 
        (mapSoundEntry.mArchiveId == 0))
    {
        return false;
    }

    const unsigned int bankEntryIndex = mapSoundEntry.mArchiveId - 1;
    const auto& bankEntries = mapFilesPair.second.GetEntries();
    if (!bankEntries.empty() && 
        (bankEntryIndex < bankEntries.size()))
    {
        const DK2SfxBankFile::BankEntry& bankEntry = bankEntries[bankEntryIndex];
        if (bankEntry.mArchiveName.empty())
        {
            cxx_assert_once(false);
            return false;
        }
        endpoint.mSoundArchive = OpenSoundArchive(bankEntry.mArchiveName);
        endpoint.mEntryIndex = mapSoundEntry.mIndex - 1;
    }

    return (endpoint.mSoundArchive != nullptr);
}

bool AudioManager::ResolveSound(const std::string& categoryName, snd_group_id groupId, SfxEndpoint& endpoint)
{
    return ResolveSound(categoryName, groupId, -1, endpoint);
}

void AudioManager::UpdateFrame(float deltaTime)
{
    if (!IsAudioOnline())
        return;

    if (mAmbienceUpdateTimer.IsStarted() &&
        mAmbienceUpdateTimer.TickAndCheckExpire(deltaTime))
    {
        mAmbienceUpdateTimer.Start();
        UpdateAmbience(false);
    }
}

bool AudioManager::PlayAmbience(const std::string& categoryName, snd_group_id groupId)
{   
    if (!IsAudioOnline())
        return false;

    if (IsAmbiencePlaying())
    {
        StopAmbience();
    }

    cxx_assert(mSoundQueueAmbience);

    // init new sequence
    const SfxFilesPair& mapFilesPair = GetSfxFilesPair(categoryName);
    unsigned int segmentsCount {};
    if (!mapFilesPair.first.GetGroupSegmentsCount(groupId, segmentsCount) || (segmentsCount == 0))
    {
        return false;
    }

    for (unsigned int isegment = 0; isegment < segmentsCount; ++isegment)
    {
        DK2SfxMapFile::SfxSoundEntry mapSoundEntry;
        if (!mapFilesPair.first.GetSoundEntry(groupId, isegment, 0, mapSoundEntry) ||
            (mapSoundEntry.mIndex == 0) || 
            (mapSoundEntry.mArchiveId == 0))
        {
            continue;
        }

        SfxEndpoint endpoint {};

        const unsigned int bankEntryIndex = mapSoundEntry.mArchiveId - 1;
        const auto& bankEntries = mapFilesPair.second.GetEntries();
        if (!bankEntries.empty() && 
            (bankEntryIndex < bankEntries.size()))
        {
            const DK2SfxBankFile::BankEntry& bankEntry = bankEntries[bankEntryIndex];
            if (bankEntry.mArchiveName.empty())
            {
                cxx_assert_once(false);
                continue;
            }
            endpoint.mSoundArchive = OpenSoundArchive(bankEntry.mArchiveName);
            endpoint.mEntryIndex = mapSoundEntry.mIndex - 1;
        }

        if (endpoint.mSoundArchive == nullptr)
            continue;

        SoLoud::AudioSource* audioSource = LoadSound(endpoint, true);
        if (audioSource == nullptr)
            continue;

        mAmbienceSequence.push_back(audioSource);
    }

    if (mAmbienceSequence.empty())
    {
        cxx_assert(false);
        return false;
    }

    mAmbienceUpdateTimer.Start(5.0f);

    UpdateAmbience(true);

    return IsAmbiencePlaying();
}

bool AudioManager::StopAmbience()
{
    if (!IsAudioOnline() || !IsAmbiencePlaying())
        return false;

    // todo: fadeout option?
    mSoundQueueAmbience->stop_queue();
    mSoundQueueAmbience->stop();

    mAmbienceUpdateTimer = {};
    mAmbienceSequencePos = 0;
    mAmbienceSequence.clear();

    return true;
}

bool AudioManager::IsAmbiencePlaying() const
{
    return mAmbienceUpdateTimer.IsStarted();
}

AudioManager::SfxFilesPair& AudioManager::GetSfxFilesPair(const std::string& categoryName)
{
    auto map_it = mCategoriesMap.find(categoryName);
    if (map_it == mCategoriesMap.end())
    {
        gConsole.LogMessage(eLogLevel_Warning, "Unknown sound category '%s'", categoryName.c_str());
    }
    // create empty if not exists
    return mCategoriesMap[categoryName];
}

void AudioManager::UpdateAmbience(bool isInitial)
{
    if (!IsAmbiencePlaying())
        return;
    
    if (isInitial)
    {
        mSoundQueueAmbience->setParamsFromAudioSource(*mAmbienceSequence.front());
        mAudioEngine->playBackground(*mSoundQueueAmbience);
    }

    const unsigned int queueCount = mSoundQueueAmbience->getQueueCount();
    for (unsigned i = queueCount; i < 2; ++i)
    {
        SoLoud::AudioSource* audioSource = mAmbienceSequence[mAmbienceSequencePos];
        mAmbienceSequencePos = (mAmbienceSequencePos + 1) % mAmbienceSequence.size(); 
        SoLoudCheckResult(mSoundQueueAmbience->play(*audioSource));
    }

    if (!isInitial && mSoundQueueAmbience->hasEnded())
    {
        mAudioEngine->playBackground(*mSoundQueueAmbience);
    }
}

SoLoud::AudioSource* AudioManager::LoadSound(const SfxEndpoint& endpoint, bool queueableAudio)
{
    cxx_assert(endpoint.mSoundArchive);
    if (endpoint.mSoundArchive == nullptr)
    {
        return nullptr;
    }

    DK2SoundArchive::SoundEntry archiveEntry;
    if (!endpoint.mSoundArchive->GetSoundEntryByIndex(endpoint.mEntryIndex, archiveEntry))
    {
        cxx_assert(false);
        return nullptr;
    }

    if (archiveEntry.mSoundType == DK2SoundArchive::eSoundType_None)
        return nullptr;

    // todo: check eSoundType_WavOld and eSoundType_Wav

    std::unique_ptr<SoLoud::AudioSource>& audioStream = mClipsCache[archiveEntry.mName];
    if (audioStream.get() == nullptr)
    {
        if (!endpoint.mSoundArchive->GetSoundEntryData(archiveEntry, mDataBuffer))
        {
            cxx_assert(false);
            return nullptr;
        }

        SoLoud::result errorCode {};
        if (queueableAudio)
        {
            std::unique_ptr<SoLoud::Wav> wavAudioStream = std::make_unique<SoLoud::Wav>();
            SoLoudCheckResult(wavAudioStream->loadMem(mDataBuffer.data(), mDataBuffer.size(), true, false));
            audioStream.reset(wavAudioStream.release());
        }
        else
        {
            std::unique_ptr<SoLoud::WavStream> wavAudioStream = std::make_unique<SoLoud::WavStream>();
            SoLoudCheckResult(wavAudioStream->loadMem(mDataBuffer.data(), mDataBuffer.size(), true, false));
            audioStream.reset(wavAudioStream.release());
        }
    }

    cxx_assert(audioStream != nullptr);
    return audioStream.get();
}

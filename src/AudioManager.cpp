#include "stdafx.h"
#include "AudioManager.h"
// SoLoud
#include "SoLoud/include/soloud.h"
#include "SoLoud/include/soloud_wavstream.h"

//////////////////////////////////////////////////////////////////////////

AudioManager gAudio;

//////////////////////////////////////////////////////////////////////////

AudioManager::AudioManager()
{

}

AudioManager::~AudioManager()
{
    cxx_assert(!mSoundEngine);
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

    gConsole.LogMessage(eLogLevel_Info, "Initialize SoLoud sound engine...");

    mSoundEngine = std::make_unique<SoLoud::Soloud>();
    SoLoud::result errorCode = mSoundEngine->init();
    if (errorCode != SoLoud::SO_NO_ERROR)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Failed to initialize sound engine: %s (%d)", mSoundEngine->getErrorString(errorCode), errorCode);
        mSoundEngine.reset();
        return false;
    }

    gConsole.LogMessage(eLogLevel_Info, "Sound engine backend string: %s", mSoundEngine->getBackendString());

    return true;
}

void AudioManager::Shutdown()
{
    for (auto& roller: mClipsCache)
    {
        roller.second->stop();
    }

    mClipsCache.clear();

    if (mSoundEngine)
    {
        mSoundEngine->deinit();
        mSoundEngine.reset();
    }

    mCategoriesMap.clear();
    mSoundArhivesMap.clear();
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

    SfxEndpoint soundLocation;
    if (!ResolveSound(categoryName, groupId, clipIndex, soundLocation))
    {
        return false;
    }

    cxx_assert(soundLocation.mSoundArchive);

    DK2SoundArchive::SoundEntry archiveEntry;
    if (!soundLocation.mSoundArchive->GetSoundEntryByIndex(soundLocation.mEntryIndex, archiveEntry))
    {
        cxx_assert(false);
        return false;
    }

    if (archiveEntry.mSoundType == DK2SoundArchive::eSoundType_None)
    {
        return false;
    }

    std::unique_ptr<SoLoud::WavStream>& audioStream = mClipsCache[archiveEntry.mName];
    if (audioStream.get() == nullptr)
    {
        audioStream = std::make_unique<SoLoud::WavStream>();
        // get clip data
        if (!soundLocation.mSoundArchive->GetSoundEntryData(archiveEntry, mDataBuffer))
        {
            cxx_assert(false);
            return false;
        }
        SoLoud::result errorCode = audioStream->loadMem(mDataBuffer.data(), mDataBuffer.size(), true, false);
        if (errorCode != SoLoud::SO_NO_ERROR)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot load sound clip '%s': %s (%d)", archiveEntry.mName.c_str(),
                mSoundEngine->getErrorString(errorCode), errorCode);
        }
    }
    cxx_assert(audioStream != nullptr);
    mSoundEngine->play(*audioStream);
    return true;
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
    return mSoundEngine.get() != nullptr;
}

bool AudioManager::ResolveSound(const std::string& categoryName, snd_group_id groupId, snd_clip_idx clipIdx, SfxEndpoint& endpoint)
{
    endpoint = {};

    auto map_it = mCategoriesMap.find(categoryName);
    if (map_it == mCategoriesMap.end())
    {
        mCategoriesMap[categoryName] = {}; // force insert empty

        gConsole.LogMessage(eLogLevel_Warning, "Unknown sound category '%s'", categoryName.c_str());
        return false;
    }

    const SfxFilesPair& mapFilesPair = map_it->second;

    // select random clip
    if (clipIdx == -1)
    {
        unsigned int soundEntriesCount {};
        if (!mapFilesPair.first.GetSoundEntriesCount(groupId, soundEntriesCount) || (soundEntriesCount == 0))
        {
            return false;
        }
        clipIdx = Random::GenerateUint(0, soundEntriesCount - 1);
    }

    DK2SfxMapFile::SfxSoundEntry mapSoundEntry;
    if (!mapFilesPair.first.GetSoundEntry(groupId, clipIdx, mapSoundEntry) ||
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

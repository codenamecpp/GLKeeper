#pragma once

//////////////////////////////////////////////////////////////////////////

// SDT File Format is for Sound Archive

//////////////////////////////////////////////////////////////////////////

class DK2SoundArchive: public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    enum eSoundType
    {
        eSoundType_None, // on blanks,
        eSoundType_Wav,
        eSoundType_WavOld, // used before 1.7, like in the german cd version
        eSoundType_Mp2Mono, // on mp2 (64kbit/s mono)
        eSoundType_Mp2Stereo, // on mp2 (112kbit/s stereo)
    };

    //////////////////////////////////////////////////////////////////////////

    struct SoundEntry
    {
        eSoundType mSoundType {};

        std::string mName;

        unsigned int mSampleRate {};
        unsigned int mDataSize {};
        unsigned int mBitsPerSample {};
        unsigned int mNumSamples {};
        unsigned int mDataOffset {};
    };

    //////////////////////////////////////////////////////////////////////////

public:
    DK2SoundArchive();
    ~DK2SoundArchive();

    bool OpenArchive(const std::string& filePath);
    void CloseArchive();

    bool IsArchiveOpened() const;

    const std::string& GetArchiveFilePath() const { return mArchiveFilePath; }

    bool GetSoundEntryByIndex(int entryIndex, SoundEntry& entry) const;
    bool GetSoundEntryByIndex(int entryIndex, SoundEntry& entry, ByteArray& entryData);
    bool GetSoundEntryDataByIndex(int entryIndex, ByteArray& entryData);
    bool GetSoundEntryData(const SoundEntry& entry, ByteArray& entryData);

    int GetSoundEntriesCount() const;

private:
    std::string mArchiveFilePath;
    std::ifstream mFileStream;
    std::vector<SoundEntry> mEntries;
};

//////////////////////////////////////////////////////////////////////////
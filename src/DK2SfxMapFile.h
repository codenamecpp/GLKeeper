#pragma once

//////////////////////////////////////////////////////////////////////////

// ***SFX.MAP File Data

//////////////////////////////////////////////////////////////////////////

class DK2SfxMapFile
{
public:
    
    //////////////////////////////////////////////////////////////////////////

    struct SfxData
    {
        unsigned int mIndex {}; // dunno what it is for
    };

    struct SfxSoundEntry
    {
        unsigned int mIndex {}; // 1 based
        unsigned int mArchiveId {}; // 1 based
    };

    struct SfxEEEntry
    {
        std::vector<SfxSoundEntry> mSoundEntries;
        std::vector<SfxData> mDataEntries;
    };

    struct SfxGroup
    {
        std::vector<SfxEEEntry> mEEEntries;
        unsigned int mGroupId {};
    };

    struct SfxMapEntry
    {
        std::vector<SfxGroup> mGroups;
        float mMinDistance {};
        float mMaxDistance {};
        float mScale = 1.0f;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    DK2SfxMapFile();

    bool OpenFile(const std::string& filePath);
    void CloseFile();
    bool IsOpened() const;

    bool GetSoundEntry(unsigned int groupId, SfxSoundEntry& entry) const;
    bool GetSoundEntry(unsigned int groupId, unsigned int index, SfxSoundEntry& entry) const;
    bool GetSoundEntriesCount(unsigned int groupId, unsigned int& entriesCount) const;

    const auto& GetEntries() const { return mSfxEntries; }
private:
    const SfxGroup* FindGroup(unsigned int groupId) const;

private:
    std::vector<SfxMapEntry> mSfxEntries;
};

//////////////////////////////////////////////////////////////////////////
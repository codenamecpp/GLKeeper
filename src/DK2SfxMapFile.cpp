#include "stdafx.h"
#include "DK2SfxMapFile.h"

DK2SfxMapFile::DK2SfxMapFile()
{
}

bool DK2SfxMapFile::OpenFile(const std::string& filePath)
{
    CloseFile();

    std::ifstream fileStream {filePath, std::ios::in | std::ios::binary};
    if (!fileStream)
        return false;

    unsigned int headerA = cxx::read_int32(fileStream);
    unsigned int headerB = cxx::read_int32(fileStream);
    unsigned int headerC = cxx::read_int32(fileStream);
    unsigned int headerD = cxx::read_int32(fileStream);

    bool hasHeaders = 
        (headerA == 0xE9612C00U) &&
        (headerB == 0x11D231D0U) &&
        (headerC == 0xB00009B4U) &&
        (headerD == 0x03F293C9U);

    cxx_assert(hasHeaders);

    if (!hasHeaders)
    {
        CloseFile();
        return false;
    }

    const unsigned int unknownData1 = cxx::read_int32(fileStream);
    const unsigned int unknownData2 = cxx::read_int32(fileStream);
    const unsigned int entriesCount = cxx::read_int32(fileStream);

    mSfxEntries.resize(entriesCount);

    for (SfxMapEntry& entriesRoller: mSfxEntries)
    {
        const unsigned int groupsCount = cxx::read_int32(fileStream);
        const unsigned int unknown1 = cxx::read_int32(fileStream);
        const unsigned int unknown2 = cxx::read_int32(fileStream);

        entriesRoller.mMinDistance = cxx::read_f32(fileStream);
        entriesRoller.mMaxDistance = cxx::read_f32(fileStream);
        entriesRoller.mScale = cxx::read_f32(fileStream);

        entriesRoller.mGroups.resize(groupsCount);

        if (!fileStream)
        {
            cxx_assert(false);

            CloseFile();
            return false;
        }
    }

    for (SfxMapEntry& entriesRoller: mSfxEntries)
    {
        for (SfxGroup& groupsRoller: entriesRoller.mGroups)
        {
            groupsRoller.mGroupId = cxx::read_int32(fileStream);

            const unsigned int groupEntriesCount = cxx::read_int32(fileStream);
            const unsigned int unknown1 = cxx::read_int32(fileStream);
            const unsigned int unknown2 = cxx::read_int32(fileStream);
            const unsigned int unknown3 = cxx::read_int32(fileStream);

            groupsRoller.mSegments.resize(groupEntriesCount);
        }

        if (!fileStream)
        {
            cxx_assert(false);

            CloseFile();
            return false;
        }
    }

    for (SfxMapEntry& entriesRoller: mSfxEntries)
    {
        for (SfxGroup& groupsRoller: entriesRoller.mGroups)
        {
            for (SfxSegment& segmentsRoller: groupsRoller.mSegments)
            {
                const unsigned int soundsCount = cxx::read_int32(fileStream);
                segmentsRoller.mSoundEntries.resize(soundsCount);

                const unsigned int soundDataCount = cxx::read_int32(fileStream);
                segmentsRoller.mDataEntries.resize(soundDataCount);

                const unsigned int unknown1 = cxx::read_int32(fileStream);
                // unknown data 26 bytes
                fileStream.seekg(26, std::ios::cur);
                
                const unsigned int unknown3 = cxx::read_int32(fileStream);
            }

            for (SfxSegment& segmentsRoller: groupsRoller.mSegments)
            {
                for (SfxSoundEntry& soundsRoller: segmentsRoller.mSoundEntries)
                {
                    soundsRoller.mIndex = cxx::read_int32(fileStream);
                    const unsigned int unknown1 = cxx::read_int32(fileStream);
                    const unsigned int unknown2 = cxx::read_int32(fileStream);
                    soundsRoller.mArchiveId = cxx::read_int32(fileStream);
                }

                for (SfxData& dataRoller: segmentsRoller.mDataEntries)
                {
                    dataRoller.mIndex = cxx::read_int32(fileStream);
                    const unsigned int unknown1 = cxx::read_int32(fileStream);
                }
            }
        }

        if (!fileStream)
        {
            cxx_assert(false);

            CloseFile();
            return false;
        }
    }

    return true;
}

void DK2SfxMapFile::CloseFile()
{
    mSfxEntries.clear();
}

bool DK2SfxMapFile::IsOpened() const
{
    return !mSfxEntries.empty();
}

bool DK2SfxMapFile::GetSoundEntry(unsigned int groupId, unsigned int segmentIndex, unsigned int index, SfxSoundEntry& entry) const
{
    if (const SfxSegment* segment = GetGroupSegment(groupId, segmentIndex))
    {
        if (!segment->mSoundEntries.empty() && 
            (segment->mSoundEntries.size() > index))
        {
            entry = segment->mSoundEntries[index];
            return true;
        }
    }
    return false;
}

bool DK2SfxMapFile::GetSoundEntriesCount(unsigned int groupId, unsigned int segmentIndex, unsigned int& entriesCount) const
{
    entriesCount = 0;
    if (const SfxSegment* segment = GetGroupSegment(groupId, segmentIndex))
    {
        entriesCount = segment->mSoundEntries.size();
    }
    return entriesCount > 0;
}

bool DK2SfxMapFile::GetGroupSegmentsCount(unsigned int groupId, unsigned int& segmentsCount) const
{
    segmentsCount = 0;
    if (const SfxGroup* group = GetGroup(groupId))
    {
        segmentsCount = group->mSegments.size();
    }
    return segmentsCount > 0;
}

const DK2SfxMapFile::SfxGroup* DK2SfxMapFile::GetGroup(unsigned int groupId) const
{
    for (const SfxMapEntry& roller: mSfxEntries)
    {
        for (const SfxGroup& groupsRoller: roller.mGroups)
        {
            if (groupsRoller.mGroupId == groupId)
                return &groupsRoller;
        }
    }
    return nullptr;
}

const DK2SfxMapFile::SfxSegment* DK2SfxMapFile::GetGroupSegment(unsigned int groupId, unsigned int segmentIndex) const
{
    if (const SfxGroup* group = GetGroup(groupId))
    {
        if (!group->mSegments.empty() && 
            (group->mSegments.size() > segmentIndex))
        {
            return &group->mSegments[segmentIndex];
        }
    }
    return nullptr;
}

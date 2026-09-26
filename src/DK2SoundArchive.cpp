#include "stdafx.h"
#include "DK2SoundArchive.h"

//////////////////////////////////////////////////////////////////////////

// https://github.com/ufdada/dk2-tools/blob/master/Formats/Sound/sdt_struct.bt

//////////////////////////////////////////////////////////////////////////

DK2SoundArchive::DK2SoundArchive()
{

}

DK2SoundArchive::~DK2SoundArchive()
{
    CloseArchive();
}

bool DK2SoundArchive::OpenArchive(const std::string& filePath)
{
    CloseArchive();

    mArchiveFilePath = filePath;
    mFileStream.open(filePath, std::ios::in | std::ios::binary);
    if (!mFileStream.is_open())
        return false;

    const unsigned int numEntries = cxx::read_int32(mFileStream);

    // get entries offsets
    std::vector<unsigned int> entriesOffsets;
    entriesOffsets.resize(numEntries);

    for (unsigned int& roller: entriesOffsets)
    {
        roller = cxx::read_int32(mFileStream);
        if (!mFileStream)
        {
            cxx_assert(false);

            CloseArchive();
            return false;
        }
    }

    mEntries.resize(numEntries);

    for (unsigned int ientry = 0; ientry < numEntries; ++ientry)
    {
        unsigned int headerSize = cxx::read_int32(mFileStream);

        SoundEntry& soundEntry = mEntries[ientry];
        soundEntry.mDataSize = cxx::read_int32(mFileStream);
        if (!cxx::read_fixed_ansi_cstring(mFileStream, 16, soundEntry.mName))
        {
            cxx_assert(false);

            CloseArchive();
            return false;
        }
        soundEntry.mSampleRate = cxx::read_int16(mFileStream);
        soundEntry.mBitsPerSample = cxx::read_int8(mFileStream);

        unsigned char soundType = cxx::read_int8(mFileStream);
        switch (soundType)
        {
            case  0: soundEntry.mSoundType = eSoundType_None; break;
            case  2: soundEntry.mSoundType = eSoundType_Wav; break;
            case  3: soundEntry.mSoundType = eSoundType_WavOld; break;
            case 36: soundEntry.mSoundType = eSoundType_Mp2Mono; break;
            case 37: soundEntry.mSoundType = eSoundType_Mp2Stereo; break;
            default:
                cxx_assert(false);
            break;
        }

        unsigned int unknown_a = cxx::read_int32(mFileStream);
        soundEntry.mNumSamples = cxx::read_int32(mFileStream);
        unsigned int unknown_b = cxx::read_int32(mFileStream);

        if (soundEntry.mDataSize > 0)
        {
            soundEntry.mDataOffset = static_cast<unsigned int>(mFileStream.tellg());
            if (!mFileStream.seekg(soundEntry.mDataSize, std::ios::cur))
            {
                cxx_assert(false);

                CloseArchive();
                return false;
            }
        }
    }

    return true;
}

void DK2SoundArchive::CloseArchive()
{
    mFileStream.close();
    mArchiveFilePath.clear();
    mEntries.clear();
}

bool DK2SoundArchive::IsArchiveOpened() const
{
    return mFileStream.is_open();
}

bool DK2SoundArchive::GetSoundEntryByIndex(int entryIndex, SoundEntry& entry) const
{
    if ((entryIndex < 0) || 
        (entryIndex >= static_cast<int>(mEntries.size())))
    {
        cxx_assert(false);
        return false;
    }
    entry = mEntries[entryIndex];
    return true;
}

bool DK2SoundArchive::GetSoundEntryByIndex(int entryIndex, SoundEntry& entry, ByteArray& entryData)
{
    bool isSuccess = GetSoundEntryByIndex(entryIndex, entry) && GetSoundEntryData(entry, entryData);
    return isSuccess;
}

bool DK2SoundArchive::GetSoundEntryDataByIndex(int entryIndex, ByteArray& entryData)
{   
    SoundEntry entry;
    return GetSoundEntryByIndex(entryIndex, entry) && GetSoundEntryData(entry, entryData);
}

bool DK2SoundArchive::GetSoundEntryData(const SoundEntry& entry, ByteArray& entryData)
{
    entryData.clear();
    if (entry.mDataSize > 0)
    {
        if (!mFileStream.seekg(entry.mDataOffset, std::ios::beg))
        {
            cxx_assert(false);
            return false;
        }
        entryData.resize(entry.mDataSize);
        if (!mFileStream.read((char*)entryData.data(), entry.mDataSize))
        {
            cxx_assert(false);
            entryData.clear();
            return false;
        }
    }
    return true;
}

int DK2SoundArchive::GetSoundEntriesCount() const
{
    return static_cast<int>(mEntries.size());
}
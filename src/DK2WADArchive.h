#pragma once

//////////////////////////////////////////////////////////////////////////
// Stores the wad file structure and contains the methods to handle the WAD
//////////////////////////////////////////////////////////////////////////

struct DK2WADArchiveEntry
{
public:
    bool mCompressed = false;
    int mDataOffset = 0;
    int mDataLength = 0;
    int mCompressedLength = 0;
};

//////////////////////////////////////////////////////////////////////////

using DK2WADArchiveEntryID = unsigned int;

//////////////////////////////////////////////////////////////////////////

class DK2WADArchive: public cxx::noncopyable
{
public:
    DK2WADArchive(const std::string& archiveName);

    const std::string& GetArchiveName() const { return mArchiveName; }

    // Open Dungeon Keeper 2 archive file
    bool OpenArchive(const std::string& theFilePath);

    // Close arhive
    void CloseArhive();

    const std::string& GetArchiveFilePath() const { return mArchiveFilePath; }

    // Find entry within wad by name
    bool FindEntryByName(const std::string& theName, DK2WADArchiveEntryID& theEntryID) const;

    // Extract entry information by its identifier
    bool ExtractEntryInfo(DK2WADArchiveEntryID entryIdentifier, DK2WADArchiveEntry& outputInfo) const;

    // Extract entry data and write to byte array
    bool ExtractEntryData(DK2WADArchiveEntryID entryIdentifier, ByteArray& theExtractData);

    // Get names of all entries within wad archive
    bool GetEntryNames(std::vector<std::string>& outNames) const;
    
private:
    //////////////////////////////////////////////////////////////////////////
    using IndicesMap = std::unordered_map<std::string, DK2WADArchiveEntryID, cxx::icase_string_hashfunc, cxx::icase_string_eq>;
    //////////////////////////////////////////////////////////////////////////
    std::string mArchiveName;
    std::string mArchiveFilePath;
    IndicesMap mIndices;
    std::vector<DK2WADArchiveEntry> mEntries;
    std::ifstream mFileStream;
};
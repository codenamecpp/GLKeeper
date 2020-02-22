#pragma once

#include "CommonTypes.h"

// game wad archive
class FileSystemArchive: public cxx::noncopyable
{
public:
    FileSystemArchive();
    ~FileSystemArchive();

    // parse and load wad archive data
    // @param archivePath: Full path
    bool OpenArchive(const std::string& archivePath);

    // unload archive data
    void CloseArchive();

    // test whether wad archive contains specific resource
    // @param resourceName: Entry name
    bool ContainsResource(const std::string& resourceName) const;

    // extract resource data from wad archive
    // @param resourceName: Entry name
    // @param outputData: Destination buffer
    bool ExtractResource(const std::string& resourceName, ByteArray& outputData) const;

private:
    void CloseWithFail(const char* errorMessage);

private:

    // file description stored within wad archive
    struct ArchiveEntryStruct
    {
    public:
        int mDataOffset;
        int mDataLength;
        int mCompressedLength;
        bool mCompressed;
    };

    using ArchiveEntriesMap = std::map<std::string, ArchiveEntryStruct, cxx::icase_less>;
    ArchiveEntriesMap mEtriesMap;

    FILE* mFileStream = nullptr;
};
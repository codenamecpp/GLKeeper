#pragma once

// game wad archive
class FileSystemArchive: public cxx::noncopyable
{
public:
    // readonly
    const std::string mName;
    const std::string mPath;

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

public:
    FileSystemArchive(const std::string& archiveName, const std::string& archivePath);
    ~FileSystemArchive();

    // parse and load wad archive data
    bool OpenArchive();

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
    FILE* mFileStream = nullptr;
};
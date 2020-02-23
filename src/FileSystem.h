#pragma once

// forwards
class FileSystemArchive;
class BinaryInputStream;

// file system manager
class FileSystem: public cxx::noncopyable
{
public:
    // readonly
    std::string mExecutablePath;
    std::string mDataPath;
    std::string mDungeonKeeperGameDataPath;
    std::string mDungeonKeeperGameRootPath;
    std::string mDungeonKeeperGameTextureCacheDirPath;
    std::vector<FileSystemArchive*> mResourceArchives;

public:

    // setup filesystem internal resources, returns false on error
    bool Initialize();
    void Deinit();

    // setup dungeon keeper game paths
    // @returns false on error
    bool SetupDungeonKeeperGamePaths();

    // mount dungeon keeper game resource archives and add them to filesystem search places
    // @returns false on error
    bool MountDungeonKeeperGameArchives();
    void UnmountDungeonKeeperGameArchives();

    // read entire text file to memory
    // @param fileName: File name
    // @param stringBuffer: Output buffer
    bool GetTextFromFile(const std::string& fileName, std::string& stringBuffer);

    // search file within data directory and resource archives and open for reading
    // @param fileName: File name
    // @return null on error
    BinaryInputStream* OpenFileStream(const std::string& fileName);

    // free file stream
    // @param fileStream: File stream
    void CloseFileStream(BinaryInputStream* fileStream);

private:
    bool SetupExecutablePath();
    bool SetupDataPath();
};

extern FileSystem gFileSystem;
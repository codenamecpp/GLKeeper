#pragma once

// forwards
class FileSystemArchive;
class BinaryInputStream;
class BinaryOutputStream;

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
    bool ReadTextFile(const std::string& fileName, std::string& stringBuffer);

    // write text to external file
    // @param fileName: File name
    // @param stringBuffer: Input buffer
    bool WriteTextFile(const std::string& fileName, const std::string& stringBuffer);

    // search file within data directory and resource archives and open for reading
    // @param fileName: File name
    // @return null on error
    BinaryInputStream* OpenDataFile(const std::string& fileName);

    // create file within data directory
    // @param fileName: File name
    // @return null on error
    BinaryOutputStream* CreateDataFile(const std::string& fileName);

    // get loaded resource archive with specific name
    // @param fileName: Archive identifier
    FileSystemArchive* FindResourceArchive(const std::string& fileName);

    // free file stream
    // @param fileStream: File stream
    void CloseFileStream(BinaryInputStream* fileStream);
    void CloseFileStream(BinaryOutputStream* fileStream);

private:
    bool SetupExecutablePath();
    bool SetupDataPath();
};

extern FileSystem gFileSystem;
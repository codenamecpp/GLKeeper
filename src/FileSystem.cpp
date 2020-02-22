#include "pch.h"
#include "FileSystem.h"
#include "Console.h"
#include "System.h"
#include "FileSystemArchive.h"
#include "BinaryInputStream.h"

FileSystem gFileSystem;

bool FileSystem::Initialize()
{
    if (!SetupExecutablePath())
    {
        Deinit();
        return false;
    }

    if (!SetupDataPath())
    {
        Deinit();
        return false;
    }

    return true;
}

void FileSystem::Deinit()
{
    UnmountDungeonKeeperGameArchives();
}

bool FileSystem::SetupExecutablePath()
{
    mExecutablePath.clear();

#if (OS_NAME == OS_WINDOWS)
    char buffer[MAX_PATH + 1];
    int count = ::GetModuleFileNameA(NULL, buffer, MAX_PATH);
    mExecutablePath.assign(buffer, (count > 0) ? count : 0);
#elif (OS_NAME == OS_LINUX)
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    mExecutablePath.assign(buffer, (count > 0) ? count : 0);
#else
    // platform not supported
    debug_assert(false);
    gConsole.LogMessage(eLogMessage_Warning, "Cannot init application executable path on current platform");
    return false;
#endif
    return true;
}

bool FileSystem::SetupDataPath()
{
    mDataPath.clear();

    filesystem::path dataPath = filesystem::path(mExecutablePath).parent_path().parent_path() / filesystem::path("data");
    if (!filesystem::is_directory(dataPath))
    {
        debug_assert(false);
        gConsole.LogMessage(eLogMessage_Warning, "Data directory does not exists");
        return false;
    }
    mDataPath = filesystem::canonical(dataPath).generic_string();
    return true;
}

bool FileSystem::SetupDungeonKeeperGamePaths()
{
    mDungeonKeeperGameDataPath.clear();
    mDungeonKeeperGameRootPath.clear();
    mDungeonKeeperGameTextureCacheDirPath.clear();

    if (gSystem.mStartupParams.mDungeonKeeperGamePath.empty())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Base game directory is not specified");
        return false;
    }

    filesystem::path basePath { gSystem.mStartupParams.mDungeonKeeperGamePath };
    mDungeonKeeperGameRootPath = basePath.generic_string();

    gConsole.LogMessage(eLogMessage_Debug, "Base game directory is '%s'", mDungeonKeeperGameRootPath.c_str());

    if (!filesystem::is_directory(basePath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Base game directory is invalid");
        return false;
    }

    filesystem::path dataPath = basePath / "Data";
    if (!filesystem::is_directory(dataPath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game data directory is invalid");
        return false;
    }

    mDungeonKeeperGameDataPath = dataPath.generic_string();
    
    filesystem::path textureCachePath = basePath / "DK2TextureCache";
    if (!filesystem::is_directory(textureCachePath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game texture cache directory is invalid");
        return false;
    }

    mDungeonKeeperGameTextureCacheDirPath = textureCachePath.generic_string();
    return true;
}

bool FileSystem::MountDungeonKeeperGameArchives()
{
    if (!filesystem::exists(mDungeonKeeperGameDataPath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game data directory is invalid");
        return false;
    }

    std::vector<filesystem::path> wads;
    const std::string archiveFileExtension { ".WAD" };

    // find all wads
    filesystem::directory_iterator iter_directory_end;
    for (filesystem::directory_iterator iter_directory(mDungeonKeeperGameDataPath); 
        iter_directory != iter_directory_end; ++iter_directory)
    {
        const filesystem::path& currentFile = iter_directory->path();
        if (filesystem::is_regular_file(currentFile))
        {
            if (currentFile.has_extension() && archiveFileExtension == currentFile.extension())
            {
                wads.push_back(currentFile);
            }
        }
    }

    if (wads.empty())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot locate game resource archives");
        return false;
    }

    gConsole.LogMessage(eLogMessage_Info, "Mounting game resource archives:");
    for (const filesystem::path& currpath: wads)
    {
        bool isSuccess = false;

        FileSystemArchive* fsArchive = new FileSystemArchive;
        if (fsArchive->OpenArchive(currpath.generic_string()))
        {
            isSuccess = true;
        }
        debug_assert(isSuccess);
        gConsole.LogMessage(eLogMessage_Info, " - %s [%s]", currpath.filename().generic_string().c_str(), isSuccess ? "OK" : "FAIL");
        if (isSuccess)
        {
            mResourceArchives.push_back(fsArchive);
            continue;
        }
        // fail
        delete fsArchive;
    }
    return true;
}

void FileSystem::UnmountDungeonKeeperGameArchives()
{
    for (FileSystemArchive* currArchive: mResourceArchives)
    {
        delete currArchive;
    }

    mResourceArchives.clear();
}

BinaryInputStream* FileSystem::OpenFileStream(const std::string& fileName)
{
    // 1 engine data path 
    filesystem::path fullFilePath = filesystem::path {mDataPath} / filesystem::path {fileName};
    if (filesystem::exists(fullFilePath))
    {
        if (filesystem::is_regular_file(fullFilePath))
        {
            FileInputStream* inputStream = new FileInputStream;
            if (inputStream->OpenFileStream(fullFilePath.generic_string()))
                return inputStream;

            debug_assert(false);
            delete inputStream;
        }
        gConsole.LogMessage(eLogMessage_Warning, "Fail to open file '%s'", fileName.c_str());
        return nullptr;
    }

    // 2 game data path
    fullFilePath = filesystem::path {mDungeonKeeperGameDataPath} / filesystem::path {fileName};
    if (filesystem::exists(fullFilePath))
    {
        if (filesystem::is_regular_file(fullFilePath))
        {
            FileInputStream* inputStream = new FileInputStream;
            if (inputStream->OpenFileStream(fullFilePath.generic_string()))
                return inputStream;

            debug_assert(false);
            delete inputStream;
        }
        gConsole.LogMessage(eLogMessage_Warning, "Fail to open file '%s'", fileName.c_str());
        return nullptr;
    }

    // 3 wads
    for (FileSystemArchive* currArchive: mResourceArchives)
    {
        if (!currArchive->ContainsResource(fileName))
            continue;

        ByteArray streamData;
        if (!currArchive->ExtractResource(fileName, streamData))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot extract file from wad archive '%s'", fileName.c_str());
            return nullptr;
        }

        MemoryInputStream* inputStream = new MemoryInputStream;
        inputStream->OpenMemoryStreamFromBuffer(streamData);
        return inputStream;
    }

    gConsole.LogMessage(eLogMessage_Warning, "File not found '%s'", fileName.c_str());
    return nullptr;
}

void FileSystem::CloseFileStream(BinaryInputStream* fileStream)
{
    debug_assert(fileStream);
    delete fileStream;
}

bool FileSystem::GetTextFromFile(const std::string& fileName, std::string& stringBuffer)
{
    BinaryInputStream* inputStream = OpenFileStream(fileName);
    if (inputStream == nullptr)
        return false;

    stringBuffer.clear();
    stringBuffer.reserve(inputStream->GetLength());

    while (!inputStream->IsEos())
    {
        unsigned char inputChar = 0;
        if (inputStream->ReadData(&inputChar, 1) == 0)
        {
            break;
        }
        
        if (inputChar == '\r' || inputChar == 0)
            continue;

        stringBuffer.push_back(inputChar);
    }

    CloseFileStream(inputStream);
    return true;
}

#include "pch.h"
#include "FileSystem.h"
#include "Console.h"
#include "System.h"
#include "FileSystemArchive.h"
#include "BinaryInputStream.h"
#include "BinaryOutputStream.h"

#if OS_NAME == OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif

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

    fs::path dataPath = fs::path(mExecutablePath).parent_path().parent_path() / fs::path("data");
    if (!fs::is_directory(dataPath))
    {
        debug_assert(false);
        gConsole.LogMessage(eLogMessage_Warning, "Data directory does not exists");
        return false;
    }
    mDataPath = fs::canonical(dataPath).generic_string();
    return true;
}

bool FileSystem::SetupDungeonKeeperGamePaths()
{
    mDungeonKeeperGameDataPath.clear();
    mDungeonKeeperGameRootPath.clear();
    mDungeonKeeperGameMapsPath.clear();
    mDungeonKeeperGameTextureCacheDirPath.clear();

    if (gSystem.mStartupParams.mDungeonKeeperGamePath.empty())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Base game directory is not specified");
        return false;
    }

    fs::path basePath { gSystem.mStartupParams.mDungeonKeeperGamePath };
    mDungeonKeeperGameRootPath = basePath.generic_string();

    gConsole.LogMessage(eLogMessage_Debug, "Base game directory is '%s'", mDungeonKeeperGameRootPath.c_str());

    if (!fs::is_directory(basePath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Base game directory is invalid");
        return false;
    }

    fs::path dataPath = basePath / "Data";
    if (!fs::is_directory(dataPath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game data directory is invalid");
        return false;
    }

    mDungeonKeeperGameDataPath = dataPath.generic_string();
    
    fs::path textureCachePath = basePath / "DK2TextureCache";
    if (!fs::is_directory(textureCachePath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game texture cache directory is invalid");
        return false;
    }

    mDungeonKeeperGameTextureCacheDirPath = textureCachePath.generic_string();

    fs::path mapsPath = dataPath / "editor/maps";
    if (!fs::is_directory(mapsPath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game maps directory is invalid");
        return false;
    }

    mDungeonKeeperGameMapsPath = mapsPath.generic_string();
    return true;
}

bool FileSystem::MountDungeonKeeperGameArchives()
{
    if (!fs::exists(mDungeonKeeperGameDataPath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game data directory is invalid");
        return false;
    }

    std::vector<fs::path> wads;
    const std::string archiveFileExtension { ".WAD" };

    // find all wads
    fs::directory_iterator iter_directory_end;
    for (fs::directory_iterator iter_directory(mDungeonKeeperGameDataPath); 
        iter_directory != iter_directory_end; ++iter_directory)
    {
        const fs::path& currentFile = iter_directory->path();
        if (fs::is_regular_file(currentFile))
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
    for (const fs::path& currpath: wads)
    {
        bool isSuccess = false;

        FileSystemArchive* fsArchive = new FileSystemArchive(currpath.filename().generic_string(), currpath.generic_string());
        if (fsArchive->OpenArchive())
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

BinaryInputStream* FileSystem::OpenDataFile(const std::string& fileName)
{
    const std::string SearchPaths[] =
    {
        mDataPath,                  // 1 engine data path 
        mDungeonKeeperGameDataPath, // 2 game data path
        mDungeonKeeperGameMapsPath  // 3 maps path
    };

    for (const std::string& currentSearchPath: SearchPaths)
    {
        fs::path fullFilePath = fs::path {currentSearchPath} / fs::path {fileName};
        if (fs::exists(fullFilePath))
        {
            if (fs::is_regular_file(fullFilePath))
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
    }

    // 4 wads
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

BinaryOutputStream* FileSystem::CreateDataFile(const std::string& fileName)
{
    fs::path filePath {fileName};
    if (filePath.is_absolute())
    {
        debug_assert(false); // forbidden
        return nullptr;
    }

    fs::path fullPath = fs::path { mDataPath } / filePath;
   
    FileOutputStream* outputStream = new FileOutputStream;
    if (outputStream->OpenFileStream(fullPath.generic_string()))
        return outputStream;

    debug_assert(false);
    delete outputStream;
    return nullptr;
}

FileSystemArchive* FileSystem::FindResourceArchive(const std::string& fileName)
{
    for (FileSystemArchive* currArchive: mResourceArchives)
    {
        if (cxx::iequals(fileName, currArchive->mName))
            return currArchive;
    }
    return nullptr;
}

void FileSystem::CloseFileStream(BinaryInputStream* fileStream)
{
    debug_assert(fileStream);
    delete fileStream;
}

void FileSystem::CloseFileStream(BinaryOutputStream* fileStream)
{
    debug_assert(fileStream);
    delete fileStream;
}

bool FileSystem::ReadTextFile(const std::string& fileName, std::string& stringBuffer)
{
    BinaryInputStream* inputStream = OpenDataFile(fileName);
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

bool FileSystem::WriteTextFile(const std::string& fileName, const std::string& stringBuffer)
{
    BinaryOutputStream* outputStream = CreateDataFile(fileName);
    if (outputStream == nullptr)
        return false;

    outputStream->WriteData(stringBuffer.data(), stringBuffer.length());
    CloseFileStream(outputStream);
    return true;
}

#include "stdafx.h"
#include "FileSystem.h"
#include <filesystem>
#include "SimplePool.h"

//////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

//////////////////////////////////////////////////////////////////////////

namespace sys = std::experimental::filesystem;

//////////////////////////////////////////////////////////////////////////

FileSystem gFiles;

//////////////////////////////////////////////////////////////////////////

bool FileSystem::Initialize()
{
    char buffer[MAX_PATH + 1] = {};
    if (::GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0)
    {
        gConsole.LogMessage(eLogLevel_Warning, "FileSystem::FileSystem(), GetModuleFileNameA Failed");
        return false;
    }

    mExecutablePath.assign(buffer);
    mWorkingDirectoryPath.assign(
        mExecutablePath.begin(), 
        mExecutablePath.begin() + mExecutablePath.find_last_of('\\'));

//#ifdef _DEBUG
    const std::string debugDataPath = FSGetParentFolder(
        FSGetParentFolder(mWorkingDirectoryPath));

    // override data path
    AddSearchPlace(debugDataPath + "/data");
    AddSearchPlace(debugDataPath + "/dungeon-keeper");
//#else
    AddSearchPlace(mWorkingDirectoryPath + "/data");
//#endif
    return true;
}

void FileSystem::Shutdown()
{
    mSearchPlaces.clear();
    mExecutablePath.clear();
    mWorkingDirectoryPath.clear();
}

bool FileSystem::LocateFont(const std::string& resourceName, std::string& resourcePath) const
{
    std::string subpath = cxx::va("fonts/%s", resourceName.c_str());
    if (PathToFile(subpath, resourcePath))
        return true;

    subpath = cxx::va("Data/Text/Default/%s", resourceName.c_str());
    return PathToFile(subpath, resourcePath);
}

bool FileSystem::LocateShader(const std::string& resourceName, std::string& resourcePath) const
{
    std::string subpath = cxx::va("shaders/%s", resourceName.c_str());
    return PathToFile(subpath, resourcePath);
}

bool FileSystem::LocateMapData(const std::string& resourceName, std::string& resourcePath) const
{
    std::string subpath;
    if (cxx::starts_with_icase(resourceName, "Data\\Editor\\") ||
        cxx::starts_with_icase(resourceName, "Data/Editor/"))
    {
        subpath = resourceName;
    }
    else
    {
        subpath = cxx::va("Data/Editor/Maps/%s", resourceName.c_str());
    }

    if (PathToFile(subpath, resourcePath))
        return true;

    subpath.append(".kwd");
    return PathToFile(subpath, resourcePath);
}

bool FileSystem::EnumMapFiles(EnumFilesCallback callback) const
{
    cxx_assert(callback);
    int filesFound = 0;
    for (const std::string& searchPlace : mSearchPlaces)
    {
        const sys::path mapsDirectory = sys::path {searchPlace} / "Data/Editor/Maps";
        const sys::path mapsExtension = ".kwd";
        if (!sys::exists(mapsDirectory))
            continue;

        sys::directory_iterator iter_directory_end;
        for (sys::directory_iterator iter_directory(mapsDirectory); 
            iter_directory != iter_directory_end; ++iter_directory)
        {
            if (!sys::is_regular_file(iter_directory->status()))
                continue;

            const sys::path& currentFile = iter_directory->path();
            if (!currentFile.has_extension() || mapsExtension != currentFile.extension())
                continue;

            callback(currentFile.stem().generic_string());
            ++filesFound;
        }
    }
    return filesFound > 0;
}

void FileSystem::AddSearchPlace(const std::string& pathToPlace)
{
    const std::string lowerPathToPlace = cxx::lower_string(pathToPlace);
    auto foundIterator = std::find_if(
        mSearchPlaces.begin(),
        mSearchPlaces.end(), [lowerPathToPlace](const std::string& stringArg)
        {
            return stringArg == lowerPathToPlace;
        });

    if (foundIterator == mSearchPlaces.end())
    {
        mSearchPlaces.emplace_front(lowerPathToPlace);
    }
}

cxx::uniqueptr<BinaryInputStream> FileSystem::OpenBinaryFile(const std::string& fileName) const
{
    static SimplePool<FileInputStream> fileInputStreamsPool;

    std::string pathToFile;

    if (PathToFile(fileName, pathToFile))
    {
        FileInputStream* fileStream = fileInputStreamsPool.Acquire();
        cxx_assert(fileStream);

        cxx::uniqueptr<BinaryInputStream> binaryStream (fileStream, 
            [](BinaryInputStream* inputStream)
            {
                if (FileInputStream* fileStream = static_cast<FileInputStream*>(inputStream))
                {
                    fileStream->CloseFileStream();
                    fileInputStreamsPool.Return(fileStream);
                }
            });

        if (fileStream->OpenFileStream(pathToFile))
            return std::move(binaryStream);

        gConsole.LogMessage(eLogLevel_Warning, "Cannot open binary file '%s'", fileName.c_str());
        return nullptr;
    }

    gConsole.LogMessage(eLogLevel_Warning, "File not found '%s'", fileName.c_str());
    return nullptr;
}

bool FileSystem::PathToFile(const std::string& fileName, std::string& fullPath) const
{
    fullPath.clear();

    for (const std::string& searchPlace : mSearchPlaces)
    {
        const sys::path pathto = sys::path {searchPlace} / fileName;
        if (sys::is_regular_file(pathto))
        {
            fullPath = pathto.generic_string();
            return true;
        }
    }
    return false;
}

bool FileSystem::PathToDirectory(const std::string& theName, std::string& fullPath) const
{
    fullPath.clear();

    for (const std::string& searchPlace : mSearchPlaces)
    {
        const sys::path pathto = sys::path {searchPlace} / theName;
        if (sys::is_directory(pathto))
        {
            fullPath = pathto.generic_string();
            return true;
        }
    } 
    return false;
}

void FSSplitPath(const std::string& filePath, 
    std::string* parentFolderPath, 
    std::string* fileNameWithoutExtension, 
    std::string* fileName, 
    std::string* fileExtension)
{
    sys::path stdFilePath{filePath};

    if (fileNameWithoutExtension)
        *fileNameWithoutExtension = stdFilePath.stem().generic_string();

    if (parentFolderPath)
        *parentFolderPath = stdFilePath.parent_path().generic_string();

    if (fileExtension)
        *fileExtension = stdFilePath.extension().generic_string();

    if (fileName)
        *fileName = stdFilePath.filename().generic_string();
}

bool FSIsDirectoryExists(const std::string & path)
{
    DWORD attributes = ::GetFileAttributes(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

bool FSIsFileExists(const std::string & path)
{
    DWORD attributes = ::GetFileAttributes(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES;
}

bool FSReadTextFromFile(const std::string& filePath, std::string& content)
{
    // clear text content
    content.clear();

    std::ifstream fileStream {filePath};
    if (!fileStream)
        return false;

    std::string stringLine {};
    while (std::getline(fileStream, stringLine, '\n'))
    {
        content.append(stringLine);
        content.append("\n");
    }

    return true;
}

bool FSReadTextLinesFromFile(const std::string& filePath, std::vector<std::string>& content)
{
    // clear text content
    content.clear();

    std::ifstream fileStream {filePath};
    if (!fileStream)
        return false;

    std::string stringLine {};
    while (std::getline(fileStream, stringLine, '\n'))
    {
        content.push_back(std::move(stringLine));
    }
    return true;
}

bool FSLoadJSON(const std::string& filePath, JsonDocument& document)
{
    std::string content;

    // parse file content
    return FSReadTextFromFile(filePath, content) && document.ParseDocument(content);
}


bool FSEnsureParentDirsExists(const std::string& path)
{
    if (path.empty())
        return false;

    std::filesystem::path srcPath {path};
    if (srcPath.has_extension()) // discard filename
    {
        srcPath = srcPath.parent_path();
    }

    if (srcPath.empty())
        return false;

    if (!std::filesystem::exists(srcPath))
    {
        std::error_code errcode;
        if (!std::filesystem::create_directories(srcPath, errcode))
            return false;
    }

    return true;
}

bool FileSystem::LocateEngineTexturesCache(std::string& thePath) const
{
    return PathToDirectory("DK2TextureCache", thePath);
}

bool FileSystem::LocateWAD(const std::string& theName, std::string& theResourcePath) const
{
    std::string subpath = cxx::va("data/%s", theName.c_str());
    return PathToFile(subpath, theResourcePath);
}

bool FileSystem::LocateTextTableFile(const std::string& fileName, std::string& resourcePath) const
{
    std::string subpath = cxx::va("Data/Text/Default/%s", fileName.c_str());
    return PathToFile(subpath, resourcePath);
}

bool FileSystem::EnumTextTableFiles(EnumFilesCallback callback) const
{
    cxx_assert(callback);
    int filesFound = 0;
    for (const std::string& searchPlace : mSearchPlaces)
    {
        const sys::path mapsDirectory = sys::path {searchPlace} / "Data/Text/Default";
        const sys::path mapsExtension = ".str";
        if (!sys::exists(mapsDirectory))
            continue;

        sys::directory_iterator iter_directory_end;
        for (sys::directory_iterator iter_directory(mapsDirectory); 
            iter_directory != iter_directory_end; ++iter_directory)
        {
            if (!sys::is_regular_file(iter_directory->status()))
                continue;

            const sys::path& currentFile = iter_directory->path();
            if (!currentFile.has_extension() || mapsExtension != currentFile.extension())
                continue;

            callback(currentFile.filename().generic_string());
            ++filesFound;
        }
    }
    return filesFound > 0;
}

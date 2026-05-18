#pragma once

#include "JsonDocument.h"
#include "BinaryInputStream.h"

//////////////////////////////////////////////////////////////////////////

using EnumFilesCallback = std::function<void(const std::string& fileName)>;

//////////////////////////////////////////////////////////////////////////
// File System
//////////////////////////////////////////////////////////////////////////

class FileSystem
{
public:
    // Setup filesystem internal resources
    bool Initialize();
    void Shutdown();

    // Find path to dungeon keeper 2 engine textures cache
    bool LocateEngineTexturesCache(std::string& thePath) const;

    // Find font resource
    bool LocateFont(const std::string& resourceName, std::string& resourcePath) const;

    // Find shader resource
    bool LocateShader(const std::string& resourceName, std::string& resourcePath) const;

    // Find map data resource
    bool LocateMapData(const std::string& resourceName, std::string& resourcePath) const;

    // Enumerate all available map files
    bool EnumMapFiles(EnumFilesCallback callback) const;

    // Find path to dungeon keeper 2 wad archive
    bool LocateWAD(const std::string& theName, std::string& theResourcePath) const;

    bool LocateTextTableFile(const std::string& fileName, std::string& resourcePath) const;

    bool EnumTextTableFiles(EnumFilesCallback callback) const;

    // Find specified file in search places and return absolute path to it
    bool PathToFile(const std::string& theName, std::string& fullPath) const;

    // Find specified directory in search places and return absolute path to it
    bool PathToDirectory(const std::string& theName, std::string& fullPath) const;

    // Add search path to file system
    void AddSearchPlace(const std::string& pathToPlace);

    // search file within data directory and resource archives and open for reading
    cxx::uniqueptr<BinaryInputStream> OpenBinaryFile(const std::string& fileName) const;

public:
    std::deque<std::string> mSearchPlaces;
    std::string mExecutablePath;
    std::string mWorkingDirectoryPath;
    std::string mDungeonKeeperContentPath;
};

extern FileSystem gFiles;

//////////////////////////////////////////////////////////////////////////

// Extact file path components
// @param filePath: Source path
void FSSplitPath(const std::string& filePath,
    std::string* parentFolderPath,
    std::string* fileNameWithoutExtension,
    std::string* fileName,
    std::string* fileExtension);

// Extract path to parent directory
// @param path: Source path
inline std::string FSGetParentFolder(const std::string& path) 
{
    std::string component;
    FSSplitPath(path, &component, nullptr, nullptr, nullptr);
    return component;
}

// Extract file name without extension
// @param path: Source path
inline std::string FSGetFileNameWithoutExtension(const std::string& path) 
{
    std::string component;
    FSSplitPath(path, nullptr, &component, nullptr, nullptr);
    return component;
}

// Extract file name including extension
// @param path: Source path
inline std::string FSGetFileName(const std::string& path) 
{
    std::string component;
    FSSplitPath(path, nullptr, nullptr, &component, nullptr);
    return component;
}

// Extract file extension (for example '.txt')
// @param path: Source path
inline std::string FSGetFileExtension(const std::string& path) 
{
    std::string component;
    FSSplitPath(path, nullptr, nullptr, nullptr, &component);
    return component;
}

// Test whether specified directory is exists
// @param path: Directory path
bool FSIsDirectoryExists(const std::string& path);

// Test whether specified file is exists
// @param path: File path
bool FSIsFileExists(const std::string& path);

// Read text content from specified file
// @param filePath: Path
// @param content: Text file content
bool FSReadTextFromFile(const std::string& filePath, std::string& content);
bool FSReadTextLinesFromFile(const std::string& filePath, std::vector<std::string>& content);

// Load json document from specified file
// @param filePath: Path
// @param document: Output document
bool FSLoadJSON(const std::string& filePath, JsonDocument& document);

bool FSEnsureParentDirsExists(const std::string& path);
#pragma once

#include "CommonTypes.h"

// binary data output stream interface
class BinaryOutputStream: public cxx::noncopyable
{
public:
    virtual ~BinaryOutputStream()
    {
    }

    // write data block to stream and advance cursor position
    // @param dataBuffer: Destination buffer
    // @param dataLength: Data size in bytes
    virtual void WriteData(const void* dataBuffer, long dataLength) = 0;
};

//////////////////////////////////////////////////////////////////////////

// file stream implementation
class FileOutputStream: public BinaryOutputStream
{
public:
    ~FileOutputStream();

    // open existing or create new file in binary mode for write
    // @param filePath: File path
    bool OpenFileStream(const std::string& filePath);
    void CloseFileStream();

    // write data block to stream and advance cursor position
    // @param dataBuffer: Destination buffer
    // @param dataLength: Data size in bytes
    void WriteData(const void* dataBuffer, long dataLength) override;

private:
    FILE* mFileStream = nullptr;
};
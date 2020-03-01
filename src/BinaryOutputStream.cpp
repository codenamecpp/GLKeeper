#include "pch.h"
#include "BinaryOutputStream.h"

FileOutputStream::~FileOutputStream()
{
    CloseFileStream();
}

bool FileOutputStream::OpenFileStream(const std::string& filePath)
{
    CloseFileStream();

    mFileStream = ::fopen(filePath.c_str(), "wb");
    if (mFileStream == nullptr)
        return false;

    return true;
}

void FileOutputStream::CloseFileStream()
{
    if (mFileStream)
    {
        ::fclose(mFileStream);
        mFileStream = nullptr;
    }
}

void FileOutputStream::WriteData(const void* dataBuffer, long dataLength)
{
    debug_assert(mFileStream);
    if (mFileStream)
    {
        fwrite(dataBuffer, dataLength, 1, mFileStream);
    }
}
#include "pch.h"
#include "BinaryInputStream.h"

FileInputStream::~FileInputStream()
{
    CloseFileStream();
}

bool FileInputStream::OpenFileStream(const std::string& filePath)
{
    CloseFileStream();

    mFileStream = ::fopen(filePath.c_str(), "rb");
    if (mFileStream == nullptr)
        return false;

    mFileLength = cxx::get_filesize(mFileStream);
    return true;
}

void FileInputStream::CloseFileStream()
{
    if (mFileStream)
    {
        ::fclose(mFileStream);
        mFileStream = nullptr;
    }
    mFileLength = 0;
    mFileCursor = 0;
}

long FileInputStream::GetCursorPosition() const
{
    debug_assert(mFileStream);
    if (mFileStream)
    {
        return cxx::get_filepos(mFileStream);
    }
    return 0;
}

bool FileInputStream::SetCursorPosition(long cursorPosition)
{
    debug_assert(mFileStream);
    if (mFileStream)
    {
        return cxx::set_filepos(mFileStream, cursorPosition);
    }
    return false;
}

bool FileInputStream::AdvanceCursorPosition(long advancePosition)
{
    debug_assert(mFileStream);
    if (mFileStream)
    {
        return cxx::advance_cursor(mFileStream, advancePosition);
    }
    return false;
}

long FileInputStream::ReadData(void* dataBuffer, long dataLength)
{
    debug_assert(mFileStream);
    debug_assert(dataBuffer);
    if (mFileStream && dataBuffer)
    {
        return ::fread(dataBuffer, 1, dataLength, mFileStream);
    }
    return 0;
}

bool FileInputStream::IsEos() const
{
    debug_assert(mFileStream);
    if (mFileStream)
    {
        return ::feof(mFileStream) != 0;
    }
    return false;
}

long FileInputStream::GetLength() const
{
    return mFileLength;
}

//////////////////////////////////////////////////////////////////////////

MemoryInputStream::~MemoryInputStream()
{
    CloseMemoryStream();
}

void MemoryInputStream::OpenMemoryStreamFromBuffer(ByteArray& sourceBuffer)
{
    CloseMemoryStream();

    mStreamData.swap(sourceBuffer);
    mStreamCursor = 0;
    mStreamLength = mStreamData.size();
}

void MemoryInputStream::CloseMemoryStream()
{
    mStreamData.clear();
    mStreamCursor = 0;
    mStreamLength = 0;
}

long MemoryInputStream::GetCursorPosition() const
{
    return mStreamCursor;
}

bool MemoryInputStream::SetCursorPosition(long cursorPosition)
{
    mStreamCursor = std::min(mStreamLength, cursorPosition);
    return true;
}

bool MemoryInputStream::AdvanceCursorPosition(long advancePosition)
{
    mStreamCursor = std::min(mStreamLength, mStreamCursor + advancePosition);
    mStreamCursor = std::max(0L, mStreamCursor);
    return true;
}

long MemoryInputStream::ReadData(void* dataBuffer, long dataLength)
{
    debug_assert(dataBuffer);

    long cursorPos = std::min(mStreamLength, mStreamCursor + dataLength);
    dataLength = (cursorPos - mStreamCursor);
    if (dataLength > 0)
    {
        memcpy(dataBuffer, mStreamData.data() + mStreamCursor, dataLength);
        mStreamCursor = cursorPos;
    }
    return dataLength;
}

bool MemoryInputStream::IsEos() const
{
    return mStreamCursor == mStreamLength;
}

long MemoryInputStream::GetLength() const
{
    return mStreamLength;
}
#pragma once

#include "CommonTypes.h"

// binary data input stream interface
class BinaryInputStream: public cxx::noncopyable
{
public:
    virtual ~BinaryInputStream()
    {
    }

    // get or set current cursor position within stream
    // @param cursorPosition: New position from start
    virtual long GetCursorPosition() const = 0;
    virtual bool SetCursorPosition(long cursorPosition) = 0;
    virtual bool AdvanceCursorPosition(long advancePosition) = 0;

    // read data block from stream and advance cursor position
    // @param dataBuffer: Destination buffer
    // @param dataLength: Data size in bytes
    // @returns num bytes in data buffer
    virtual long ReadData(void* dataBuffer, long dataLength) = 0;

    // test whether it is end of stream reached
    virtual bool IsEos() const = 0;

    // get binary stream size in bytes
    virtual long GetLength() const = 0;
};

//////////////////////////////////////////////////////////////////////////

// file stream implementation
class FileInputStream: public BinaryInputStream
{
public:
    ~FileInputStream();

    // open file stream in binary mode for reading
    // @param filePath: File path
    bool OpenFileStream(const std::string& filePath);
    void CloseFileStream();

    // get or set current cursor position within stream
    // @param cursorPosition: New position from start
    long GetCursorPosition() const override;
    bool SetCursorPosition(long cursorPosition) override;
    bool AdvanceCursorPosition(long advancePosition) override;

    // read data block from stream and advance cursor position
    // @param dataBuffer: Destination buffer
    // @param dataLength: Data size in bytes
    // @returns num bytes in data buffer
    long ReadData(void* dataBuffer, long dataLength) override;

    // test whether it is end of stream reached
    bool IsEos() const override;

    // get binary stream size in bytes
    long GetLength() const override;

private:
    FILE* mFileStream = nullptr;
    long mFileLength = 0;
    long mFileCursor = 0;
};

//////////////////////////////////////////////////////////////////////////

// memory stream implementation
class MemoryInputStream: public BinaryInputStream
{
public:
    ~MemoryInputStream();

    // setup stream, moves source buffer data into internal buffer (takes ownership)
    void OpenMemoryStreamFromBuffer(ByteArray& sourceBuffer);
    void CloseMemoryStream();

    // get or set current cursor position within stream
    // @param cursorPosition: New position from start
    long GetCursorPosition() const override;
    bool SetCursorPosition(long cursorPosition) override;
    bool AdvanceCursorPosition(long advancePosition) override;

    // read data block from stream and advance cursor position
    // @param dataBuffer: Destination buffer
    // @param dataLength: Data size in bytes
    // @returns num bytes in data buffer
    long ReadData(void* dataBuffer, long dataLength) override;

    // test whether it is end of stream reached
    bool IsEos() const override;

    // get binary stream size in bytes
    long GetLength() const override;

private:
    ByteArray mStreamData;
    long mStreamLength = 0;
    long mStreamCursor = 0;
};
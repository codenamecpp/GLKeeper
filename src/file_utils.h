#pragma once

namespace cxx
{
    // simple unique_ptr for file
    struct file_unique_ptr
    {
    public:
        file_unique_ptr() {}
        file_unique_ptr(const char* filepath, const char* filemode) 
        {
            mFileStream = ::fopen(filepath, filemode);
        }
        ~file_unique_ptr()
        {
            if (mFileStream)
            {
                ::fclose(mFileStream);
                mFileStream = nullptr;
            }
        }
    public:
        FILE* mFileStream = nullptr;        
    };

    // read data from file stream to output buffer
    // @param stream: File stream
    // @param buffer: Output buffer
    // @param bufferLength: Size of output buffer in bytes
    inline bool read_data(FILE* stream, void* buffer, int bufferLength)
    {
        debug_assert(stream);
        debug_assert(buffer);
        size_t num_elements = ::fread(buffer, bufferLength, 1, stream);
        return (num_elements == 1);
    }

    // read int32 data from file stream
    // @param stream: File stream
    // @param outputData: Output int32 data
    inline bool read_int32(FILE* stream, int& outputData)
    {
        debug_assert(stream);
        size_t num_elements = ::fread(&outputData, sizeof(int), 1, stream);
        return (num_elements == 1);
    }

    // read uint32 data from file stream
    // @param stream: File stream
    // @param outputData: Output uint32 data
    inline bool read_uint32(FILE* stream, unsigned int& outputData)
    {
        debug_assert(stream);
        size_t num_elements = ::fread(&outputData, sizeof(unsigned int), 1, stream);
        return (num_elements == 1);
    }

    // read int16 data from file stream
    // @param stream: File stream
    // @param outputData: Output int16 data
    inline bool read_int16(FILE* stream, short& outputData)
    {
        debug_assert(stream);
        size_t num_elements = ::fread(&outputData, sizeof(short), 1, stream);
        return (num_elements == 1);
    }

    // read uint16 data from file stream
    // @param stream: File stream
    // @param outputData: Output uint16 data
    inline bool read_uint16(FILE* stream, unsigned short& outputData)
    {
        debug_assert(stream);
        size_t num_elements = ::fread(&outputData, sizeof(unsigned short), 1, stream);
        return (num_elements == 1);
    }

    // read int8 data from file stream
    // @param stream: File stream
    // @param outputData: Output int8 data
    inline bool read_int8(FILE* stream, char& outputData)
    {
        debug_assert(stream);
        size_t num_elements = ::fread(&outputData, sizeof(char), 1, stream);
        return (num_elements == 1);
    }

    // read uint8 data from file stream
    // @param stream: File stream
    // @param outputData: Output uint8 data
    inline bool read_uint8(FILE* stream, unsigned char& outputData)
    {
        debug_assert(stream);
        size_t num_elements = ::fread(&outputData, sizeof(unsigned char), 1, stream);
        return (num_elements == 1);
    }

    // set cursor position within file stream
    // @param stream: File stream
    // @param position: Position from start in bytes
    inline bool set_filepos(FILE* stream, long position)
    {
        debug_assert(stream);
        return ::fseek(stream, position, SEEK_SET) == 0;
    }

    // get current cursor position within file stream
    // @param stream: File stream
    inline long get_filepos(FILE* stream)
    {
        debug_assert(stream);
        return ::ftell(stream);
    }

    // advance cursor position within file stream
    // @param stream: File stream
    // @parem bytesCount: Skip bytes count 
    inline bool advance_cursor(FILE* stream, long bytesCount)
    {
        debug_assert(stream);
        return ::fseek(stream, bytesCount, SEEK_CUR) == 0;
    }

    // get file size in bytes
    // @param stream: File stream
    inline long get_filesize(FILE* stream)
    {
        debug_assert(stream);
        long startPosition = ::ftell(stream);
        ::fseek(stream, 0, SEEK_END);
        long fileSize = ::ftell(stream);
        ::fseek(stream, startPosition, SEEK_SET);
        return fileSize;
    }

} // namespace cxx

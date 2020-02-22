#include "pch.h"
#include "FileSystemArchive.h"
#include "Console.h"

//////////////////////////////////////////////////////////////////////////

const unsigned char WAD_HEADER_IDENTIFIER[] = {'D', 'W', 'F', 'B'};
const int WAD_HEADER_VER = 2;
const int WAD_OFFSET = 0x48;

//////////////////////////////////////////////////////////////////////////

class file_stream_range
{
public:
    file_stream_range(FILE* fileStream, int dataPosition, int dataLength)
        : mFileStream(fileStream)
        , mCursor()
        , mLength(dataLength)
    {
        cxx::set_filepos(mFileStream, dataPosition);
    }

    inline unsigned char get_byte()
    {
        unsigned char buffer = 0;
        if (mCursor < mLength)
        {
            bool isSuccess = cxx::read_uint8(mFileStream, buffer);
            if (isSuccess)
            {
                ++mCursor;
            }
            debug_assert(isSuccess);
        }
        return buffer;
    }

    inline bool is_eof() const { return mCursor == mLength; }

private:
    FILE* mFileStream;
    int mCursor;
    int mLength;
};

//////////////////////////////////////////////////////////////////////////

// Thanks to Trass3r for his decompression algorithm.

static bool dk2_wad_decompress_data(file_stream_range& input, int compressedLength, unsigned char* theBuffer)
{
    bool isFinished = false;
    if (input.get_byte() & 1) 
    {
        input.get_byte();
        input.get_byte();
        input.get_byte();
    }
    input.get_byte();

    // decompressed data length
    const int decsize = (input.get_byte() << 16) + (input.get_byte() << 8) + input.get_byte();
    debug_assert(decsize == compressedLength);
    if (decsize != compressedLength)
    {
        return false;
    }

    int j = 0;
    unsigned char flag; // The flag byte read at the beginning of each main loop iteration
    int counter = 0; // Counter for all loops

    // process
    for (; !isFinished;)
    {
        if (input.is_eof())
            break;

        flag = input.get_byte(); // Get flag byte
        if ((flag & 0x80) == 0) 
        {
            unsigned char tmp = input.get_byte();
            counter = flag & 3; // mod 4
            while (counter-- != 0) // Copy literally
            {
                theBuffer[j] = input.get_byte();
                j++;
            }
            int k = j; // Get the destbuf position
            k -= (flag & 0x60) << 3;
            k -= tmp;
            k--;

            counter = ((flag >> 2) & 7) + 2;
            do 
            {
                theBuffer[j] = theBuffer[k++];
                j++;
            } while (counter-- != 0); // Correct decrement
        }
        else if ((flag & 0x40) == 0) 
        {
            unsigned char tmp = input.get_byte();
            unsigned char tmp2 = input.get_byte();
            counter = tmp >> 6;
            while (counter-- != 0) // Copy literally
            {
                theBuffer[j] = input.get_byte();
                j++;
            }
            int k = j;
            k -= (tmp & 0x3F) << 8;
            k -= tmp2;
            k--;
            counter = (flag & 0x3F) + 3;
            do {
                theBuffer[j] = theBuffer[k++];
                j++;
            } while (counter-- != 0); // Correct postfix decrement
        }
        else if ((flag & 0x20) == 0) 
        {
            unsigned char localtemp = input.get_byte();
            unsigned char tmp2 = input.get_byte();
            unsigned char tmp3 = input.get_byte();
            counter = flag & 3;
            while (counter-- != 0) // Copy literally
            {
                theBuffer[j] = input.get_byte();
                j++;
            }
            int k = j;
            k -= (flag & 0x10) << 12;
            k -= localtemp << 8;
            k -= tmp2;
            k--;
            counter = tmp3 + ((flag & 0x0C) << 6) + 4;
            do {
                theBuffer[j] = theBuffer[k++];
                j++;
            } while (counter-- != 0); // Correct
        } 
        else 
        {
            counter = (flag & 0x1F) * 4 + 4;
            if (counter > 0x70) 
            {
                isFinished = true;
                // Prepare to copy the last bytes
                counter = flag & 3;
            }
            while (counter-- != 0) // Copy literally
            {
                theBuffer[j] = input.get_byte();
                j++;
            }
        }
    }
    return isFinished;
}

//////////////////////////////////////////////////////////////////////////

FileSystemArchive::FileSystemArchive()
{
}

FileSystemArchive::~FileSystemArchive()
{
    CloseArchive();
}

bool FileSystemArchive::OpenArchive(const std::string& archivePath)
{
    CloseArchive();

    mFileStream = fopen(archivePath.c_str(), "rb");
    if (mFileStream == nullptr)
        return false;

    // reading header
    unsigned char headerSignature[sizeof(WAD_HEADER_IDENTIFIER)];
    if (!cxx::read_data(mFileStream, headerSignature, sizeof(WAD_HEADER_IDENTIFIER)))
    {
        CloseWithFail("Cannot read archive signature");
        return false;
    }

    if (memcmp(headerSignature, WAD_HEADER_IDENTIFIER, sizeof(WAD_HEADER_IDENTIFIER)))
    {
        CloseWithFail("Archive signature invalid");
        return false;
    }

    int headerVersion = 0;
    if (!cxx::read_int32(mFileStream, headerVersion) || headerVersion != WAD_HEADER_VER)
    {
        CloseWithFail("Cannot read archive format version");
        return false;
    }

    cxx::set_filepos(mFileStream, WAD_OFFSET);

    // reading data
    int numEntries;
    int nameOffset;
    int nameSize;
    int unknown;

    if (!cxx::read_int32(mFileStream, numEntries) || !cxx::read_int32(mFileStream, nameOffset) ||
        !cxx::read_int32(mFileStream, nameSize) || !cxx::read_int32(mFileStream, unknown))
    {
        CloseWithFail("Cannot read archive entries info");
        return false;
    }

    long entriesStartCursor = cxx::get_filepos(mFileStream);

    // read entry names
    std::vector<char> entrynames;
    entrynames.resize(nameSize);

    cxx::set_filepos(mFileStream, nameOffset);
    if (!cxx::read_data(mFileStream, entrynames.data(), nameSize))
    {
        CloseWithFail("Cannot read archive names");
        return false;
    }

    cxx::set_filepos(mFileStream, entriesStartCursor);

    // reading entries
    for (int ifile = 0, inameoffset = 0; ifile < numEntries; ++ifile)
    {   
        cxx::advance_cursor(mFileStream, sizeof(int));

        // read file entry data
        int fileNameOffset;
        int fileNameLength;
        if (!cxx::read_int32(mFileStream, fileNameOffset) || !cxx::read_int32(mFileStream, fileNameLength))
        {
            CloseWithFail("Cannot read archive entry data");
            return false;
        }

        int fileDataOffset;
        int fileDataLength;
        int fileCompression;
        int fileCompressedDataLength;
        if (!cxx::read_int32(mFileStream, fileDataOffset) || !cxx::read_int32(mFileStream, fileDataLength) ||
            !cxx::read_int32(mFileStream, fileCompression) || !cxx::read_int32(mFileStream, fileCompressedDataLength))
        {
            CloseWithFail("Cannot read archive entry data");
            return false;
        }

        // skip unknown data
        cxx::advance_cursor(mFileStream, sizeof(int));
        cxx::advance_cursor(mFileStream, sizeof(int));
        cxx::advance_cursor(mFileStream, sizeof(int));

        std::string entryName(&entrynames[inameoffset]);

        // set information
        ArchiveEntryStruct& archiveEntry = mEtriesMap[entryName];
        archiveEntry.mCompressedLength = fileCompressedDataLength;
        archiveEntry.mDataLength = fileDataLength;
        archiveEntry.mDataOffset = fileDataOffset;
        archiveEntry.mCompressed = (fileCompression == 4);
        inameoffset += fileNameLength;
    }
    return true;
}

void FileSystemArchive::CloseArchive()
{
    if (mFileStream)
    {
        fclose(mFileStream);
        mFileStream = nullptr;
    }
    mEtriesMap.clear();
}

bool FileSystemArchive::ContainsResource(const std::string& resourceName) const
{
    debug_assert(mFileStream);

    auto resource_iterator = mEtriesMap.find(resourceName);
    if (resource_iterator == mEtriesMap.end())
        return false;

    return true;
}

bool FileSystemArchive::ExtractResource(const std::string& resourceName, ByteArray& theExtractData) const
{
    debug_assert(mFileStream);
    if (mFileStream == nullptr)
        return false;

    auto resource_iterator = mEtriesMap.find(resourceName);
    if (resource_iterator == mEtriesMap.end())
        return false;

    const ArchiveEntryStruct& archiveEntry = resource_iterator->second;

    theExtractData.clear();
    theExtractData.resize((archiveEntry.mCompressed) ? archiveEntry.mCompressedLength : archiveEntry.mDataLength);

    if (!cxx::set_filepos(mFileStream, archiveEntry.mDataOffset))
    {
        debug_assert(false);
        return false;
    }

    // read uncompressed data
    bool isCompressed = archiveEntry.mCompressed;
    if (!isCompressed)
    {
        if (!cxx::read_data(mFileStream, theExtractData.data(), archiveEntry.mDataLength))
        {
            debug_assert(false);
            return false;
        }
        return true;
    }

    // read compressed data
    file_stream_range input{mFileStream, archiveEntry.mDataOffset, archiveEntry.mDataLength};
    return dk2_wad_decompress_data(input, archiveEntry.mCompressedLength, theExtractData.data());
}

void FileSystemArchive::CloseWithFail(const char* errorMessage)
{
    debug_assert(false);
    CloseArchive();

    gConsole.LogMessage(eLogMessage_Debug, errorMessage);
}

#include "stdafx.h"
#include "DK2WADArchive.h"

//////////////////////////////////////////////////////////////////////////

// https://github.com/tonihele/OpenKeeper/blob/master/src/toniarts/openkeeper/tools/convert/wad/WadFile.java

//////////////////////////////////////////////////////////////////////////

static const unsigned char WAD_HEADER_IDENTIFIER[] = {'D', 'W', 'F', 'B'};

enum 
{ 
    WAD_HEADER_VER = 2,
    WAD_OFFSET = 0x48
};

//////////////////////////////////////////////////////////////////////////
// Imlementation details

//////////////////////////////////////////////////////////////////////////

enum dk2_wad_entry_compression
{
    dk2_wad_entry_compression_none,
    dk2_wad_entry_compression_compressed,
    dk2_wad_entry_compression_unknown
};

//////////////////////////////////////////////////////////////////////////

class file_stream_range
{
public:
    file_stream_range(std::ifstream& theStream, int dataPosition, int dataLength)
        : mStream(theStream)
        , mCursor()
        , mLength(dataLength)
    {
        mStream.seekg(dataPosition, std::ios::beg);
    }

    inline unsigned char get_byte()
    {
        unsigned char buffer;
        if (mCursor < mLength)
        {
            ++mCursor;
            bool isSuccess = cxx::read_elements(mStream, &buffer, 1);
            cxx_assert(isSuccess);
            if (isSuccess)
            {
                return buffer;
            }
        }
        return 0;
    }

    inline bool is_eof() const { return mCursor == mLength; }

private:
    std::ifstream& mStream;
    int mCursor;
    int mLength;
};

//////////////////////////////////////////////////////////////////////////

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
    cxx_assert(decsize == compressedLength);
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
        {
            break;
        }

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

DK2WADArchive::DK2WADArchive(const std::string& archiveName)
    : mArchiveName(archiveName)
{
}

bool DK2WADArchive::OpenArchive(const std::string& theFilePath)
{
    CloseArhive();
    mFileStream.open(theFilePath, std::ios::in | std::ios::binary);
    if (!mFileStream.is_open())
        return false;

    // reading header
    unsigned char headerSignature[sizeof(WAD_HEADER_IDENTIFIER)] = {};
    mFileStream.read((char*)headerSignature, sizeof(WAD_HEADER_IDENTIFIER));
    if (memcmp(headerSignature, WAD_HEADER_IDENTIFIER, sizeof(WAD_HEADER_IDENTIFIER)))
    {
        // invalid header
        CloseArhive();
        return false;
    }

    int headerVersion = 0;
    cxx::read_elements(mFileStream, &headerVersion, 1);
    if (headerVersion != WAD_HEADER_VER)
    {
        // invalid version
        CloseArhive();
        return false;
    }
       
    mFileStream.seekg(WAD_OFFSET, std::ios::beg);

    // reading data
    const int numEntries = cxx::read_int32(mFileStream);
    const int nameOffset = cxx::read_int32(mFileStream);
    const int nameSize = cxx::read_int32(mFileStream);
    const int unknown = cxx::read_int32(mFileStream);

    const std::streamoff entriesStartCursor = mFileStream.tellg();

    // read file names
    std::vector<unsigned char> entrynames;
    entrynames.resize(nameSize);
    mFileStream.seekg(nameOffset, std::ios::beg);
    if (!mFileStream.read((char*)&entrynames[0], nameSize))
    {
        // cannot read file names
        CloseArhive();
        return false;
    }

    // allocate entries
    mEntries.resize(numEntries);
    mFileStream.seekg(entriesStartCursor, std::ios::beg);

    // reading entries
    for (int ifile = 0, inameoffset = 0; ifile < numEntries; ++ifile)
    {
        cxx::read_int32(mFileStream);// skip data

        // read file entry data
        const int fileNameOffset = cxx::read_int32(mFileStream);
        const int fileNameLength = cxx::read_int32(mFileStream);
        const int fileDataOffset = cxx::read_int32(mFileStream);
        const int fileDataLength = cxx::read_int32(mFileStream);
        const int fileCompression = cxx::read_int32(mFileStream);
        const int fileCompressedDataLength = cxx::read_int32(mFileStream);

        // skip unknown data
        cxx::read_int32(mFileStream);
        cxx::read_int32(mFileStream);
        cxx::read_int32(mFileStream);

        const std::string entryName((const char*)&entrynames[inameoffset]);

        // set information
        mEntries[ifile].mCompressedLength = fileCompressedDataLength;
        mEntries[ifile].mDataLength = fileDataLength;
        mEntries[ifile].mDataOffset = fileDataOffset;
        mEntries[ifile].mCompressed = (fileCompression == 4);
        mIndices[entryName] = ifile;
        inameoffset += fileNameLength;
    }

    mArchiveFilePath = theFilePath;
    return true;
}

bool DK2WADArchive::FindEntryByName(const std::string& theName, DK2WADArchiveEntryID& theEntryID) const
{
    auto found_iterator = mIndices.find(theName);
    if (found_iterator != mIndices.end())
    {
        theEntryID = found_iterator->second;
        return true;
    }
    return false;
}

bool DK2WADArchive::ExtractEntryInfo(DK2WADArchiveEntryID entryIdentifier, DK2WADArchiveEntry& outputInfo) const
{
    cxx_assert(entryIdentifier < mEntries.size());
    if (entryIdentifier < mEntries.size())
    {
        outputInfo = mEntries[entryIdentifier];
        return true;
    }
    return false;
}

bool DK2WADArchive::ExtractEntryData(DK2WADArchiveEntryID entryIdentifier, ByteArray& theExtractData)
{
    cxx_assert(entryIdentifier < mEntries.size());
    if (entryIdentifier < mEntries.size())
    {
        theExtractData.clear();
        theExtractData.resize((mEntries[entryIdentifier].mCompressed) ? 
            mEntries[entryIdentifier].mCompressedLength : mEntries[entryIdentifier].mDataLength);

        // set file pointer
        mFileStream.seekg(mEntries[entryIdentifier].mDataOffset, std::ios::beg);

        // read uncompressed data
        bool isCompressed = mEntries[entryIdentifier].mCompressed;
        if (!isCompressed)
        {
            if (!mFileStream.read((char*)&theExtractData[0], mEntries[entryIdentifier].mDataLength))
                return false;

            return true;
        }
        // read compressed data
        file_stream_range input{mFileStream, mEntries[entryIdentifier].mDataOffset, mEntries[entryIdentifier].mDataLength};
        return dk2_wad_decompress_data(input, mEntries[entryIdentifier].mCompressedLength, &theExtractData[0]);
    }
    return false;
}


bool DK2WADArchive::GetEntryNames(std::vector<std::string>& outNames) const
{
    outNames.clear();
    for (const auto& roller: mIndices)
    {
        outNames.push_back(roller.first);
    }
    return !outNames.empty();
}

void DK2WADArchive::CloseArhive()
{
    mEntries.clear();
    mIndices.clear();
    mFileStream.close();
    mArchiveFilePath.clear();
}

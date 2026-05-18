#include "stdafx.h"
#include "DK2Font.h"

static const unsigned char BF4_HEADER_IDENTIFIER[] = {'F', '4', 'F', 'B'};

enum BF4_Encoding
{
    BF4_Encoding_None = 0,
    BF4_Encoding_RLE4 = 1,
    BF4_Encoding_RLE1 = 2,
};

static const unsigned char g_palette_levels[16] =
{
    0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255
};

//////////////////////////////////////////////////////////////////////////

class stream_4_bits_writer
{
public:
    stream_4_bits_writer(unsigned char* theBuffer, int theBufferLength)
        : mBuffer(theBuffer)
        , mCursorPos()
        , mCounter()
        , mBufferLength(theBufferLength)
    {}

    inline void write(unsigned char theData)
    {
        if ((mCounter % 2) == 0)
        {
            cxx_assert(mCursorPos < mBufferLength);
            mBuffer[mCursorPos] = (theData & 0x0F) << 4;
        }
        else
        {
            mBuffer[mCursorPos] |= (theData & 0x0F);
            ++mCursorPos;
        }
        ++mCounter;
    }

    inline int get_cursor() const { return mCursorPos; }

private:
    unsigned char* mBuffer;
    int mCursorPos; // byte
    int mCounter;
    int mBufferLength;
};

class stream_4_bits_reader
{
public:
    stream_4_bits_reader(std::istream& theStream, int theSourceLength)
        : mFileStream(theStream)
        , mSourceLength(theSourceLength)
        , mCursorPos()
        , mByte()
        , mCounter()
    {}

    inline unsigned char read()
    {
        unsigned char fourBits = 0;
        if ((mCounter % 2) == 0)
        {
            mByte = cxx::read_int8(mFileStream);
            fourBits = (mByte & 0xF0) >> 4;
        }
        else
        {
            fourBits = (mByte & 0x0F);
            ++mCursorPos;
            cxx_assert(mCursorPos <= mSourceLength);
        }
        ++mCounter;
        return fourBits;
    }

    inline int get_cursor() const { return mCursorPos; }

private:
    std::istream& mFileStream;
    int mSourceLength;
    int mCursorPos;
    unsigned char mByte;
    int mCounter;
};

//////////////////////////////////////////////////////////////////////////

static bool dk2_bf4_decode_RLE4(std::istream& theInputStream, int theSourceLength, unsigned char* theBuffer, int theBufferLength)
{
    stream_4_bits_writer four_bits_writer { theBuffer, theBufferLength };
    stream_4_bits_reader four_bits_reader { theInputStream, theSourceLength };

    for (;;)
    {
        unsigned char value = four_bits_reader.read();

        if (value)
        {
            // just write it
            four_bits_writer.write(value);
        }
        else
        {
            int count = four_bits_reader.read();
            if (count)
            {
                value = four_bits_reader.read();
                for (int i = 0; i < count; ++i)
                {
                    four_bits_writer.write(value);
                }
            }
            else
            {
                break; // end of stream
            }
        }
    } // for
    return true;
}

static bool dk2_bf4_read_entry(std::istream& theStream, DK2FontDesc* theFont, DK2FontEntry* theEntry)
{
    theEntry->mCharacter = cxx::read_int16(theStream);
    int uUnknown = cxx::read_int16(theStream);
    int dataSize = cxx::read_int32(theStream);
    int compressedSize = cxx::read_int32(theStream);
    unsigned char fontEncoding = cxx::read_int8(theStream);
    // skip
    unsigned char uByte = cxx::read_int8(theStream);
    uByte = cxx::read_int8(theStream);
    uByte = cxx::read_int8(theStream);

    theEntry->mWidth = cxx::read_int16(theStream);
    theEntry->mHeight = cxx::read_int16(theStream);
    theEntry->mOffsetX = (char)cxx::read_int8(theStream);//signed
    theEntry->mOffsetY = (char)cxx::read_int8(theStream);//signed
    theEntry->mOuterWidth = cxx::read_int16(theStream);

    // glyph is empty
    if (dataSize < 1 || theEntry->mWidth < 1 || theEntry->mHeight < 1)
        return true;

    int pixelsCount = theEntry->mWidth * theEntry->mHeight;
    std::vector<unsigned char> sourcePixels;
    int sourceLength = 0;

    if (fontEncoding == BF4_Encoding_RLE1) // seems does not used ?
    {
        cxx_assert("Not supported yet");
        return false;
    }
    else if (fontEncoding == BF4_Encoding_RLE4)
    {
        sourcePixels.resize(compressedSize);
        sourceLength = compressedSize;
        bool isDecoded = dk2_bf4_decode_RLE4(theStream, dataSize, sourcePixels.data(), compressedSize);
        cxx_assert(isDecoded);
        if (!isDecoded)
            return false;
    }
    else if (fontEncoding == BF4_Encoding_None)
    {
        sourcePixels.resize(dataSize);
        sourceLength = dataSize;
        if (!theStream.read((char*) sourcePixels.data(), dataSize))
        {
            cxx_assert(false);
            return false;
        }
    }
    else
    {
        cxx_assert(!"BF4_UNKNOWN_ENCODING");
        return false;
    }

    bool isBadLength = sourceLength * 2 < theEntry->mWidth * theEntry->mHeight;
    cxx_assert(!isBadLength);
    if (isBadLength)
        return false;

    // unpack pixels
    theEntry->mImage.resize(pixelsCount);
    for (int ipixel = 0, icursor = 0; ipixel < pixelsCount; ++ipixel)
    {
        if (ipixel % 2 == 0)
        {
            unsigned char udata = sourcePixels[icursor]; // do not advance cursor here
            unsigned char uindex = ((udata & 0xF0) >> 4) & 0x0F;
            theEntry->mImage[ipixel] = g_palette_levels[uindex];
        }
        else
        {
            unsigned char udata = sourcePixels[icursor++]; // advance cursor here
            unsigned char uindex = udata & 0x0F;
            theEntry->mImage[ipixel] = g_palette_levels[uindex];
        }
    }

    if (theFont->mMaxWidth < theEntry->mWidth)
        theFont->mMaxWidth = theEntry->mWidth;

    if (theFont->mMaxHeight < theEntry->mHeight)
        theFont->mMaxHeight = theEntry->mHeight;

    if (theFont->mMaxCodePoint < theEntry->mCharacter)
        theFont->mMaxCodePoint = theEntry->mCharacter;

    theFont->mAvgWidth += theEntry->mWidth;
    return true;
}

bool DK2_BF4_LoadFromStream(std::istream& fileStream, DK2FontDesc& theFont)
{
    if (!fileStream)
        return false;
    
    // read header
    unsigned char headerSignature[sizeof(BF4_HEADER_IDENTIFIER)] = {};
    if (!fileStream.read((char*)headerSignature, sizeof(BF4_HEADER_IDENTIFIER)))
        return false;

    if (memcmp(headerSignature, BF4_HEADER_IDENTIFIER, sizeof(BF4_HEADER_IDENTIFIER)))
    {
        // invalid header
        return false;
    }
    theFont = {};
    theFont.mMaxWidth = cxx::read_int8(fileStream);
    theFont.mMaxHeight = cxx::read_int8(fileStream);
    int numEntries = cxx::read_int16(fileStream);

    // collect entries
    theFont.mEntries.resize(numEntries);
    for (int ientry = 0; ientry < numEntries; ++ientry)
    {
        int offset = cxx::read_int32(fileStream);
        // read element data
        std::streampos currentStreamPos = fileStream.tellg();
        fileStream.seekg(offset, std::ios::beg);
        bool isok = dk2_bf4_read_entry(fileStream, &theFont, &theFont.mEntries[ientry]);
        cxx_assert(isok);
        fileStream.seekg(currentStreamPos, std::ios::beg);
    }
    theFont.mAvgWidth /= numEntries;
    return true;
}
#pragma once

//////////////////////////////////////////////////////////////////////////

// BMFont .fnt binary format description : http://www.angelcode.com/products/bmfont/doc/file_format.html
enum 
{
    BMFONT_BLOCKID_INFO         = 1,
    BMFONT_BLOCKID_COMMON       = 2,
    BMFONT_BLOCKID_PAGES        = 3,
    BMFONT_BLOCKID_CHARS        = 4,
    BMFONT_BLOCKID_KERNINGPAIRS = 5,
    NUM_BMFONT_BLOCKS // should be very last
};

enum 
{
    BMFONT_MAX_FONT_NAME_LENGTH = 64,
    BMFONT_MAX_TEXTURE_NAME_LENGTH = 64
};

enum 
{
    BMFONT_VERSION = 3
};

enum 
{
    BMFONT_NUMCHARS = 256
};

//////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct BMFontHeader
{
public:
    char signature[3]; // 42 4D 46, "BMF"
    unsigned char version; // 3
};
enum { BMFONT_SIGNATURE = 0x03464D42 };

struct BMFontBlockHeader
{
public:
    unsigned char mID; // BlockID_xxx values
    unsigned int mLength;
};

struct BMFontBlockInfo
{
public:
    signed short mLength;
    unsigned char mBits;
    unsigned char mCharset;
    unsigned short mStretchH;
    unsigned char mAA;
    unsigned char mPaddingUp;
    unsigned char mPaddingRight;
    unsigned char mPaddingDown;
    unsigned char mPaddingLeft;
    unsigned char mSpacingHorz;
    unsigned char mSpacingVert;
    unsigned char mOutline;
};

struct BMFontBlockInfoEx
{
public:
    char mFontName[BMFONT_MAX_FONT_NAME_LENGTH];
    char mTextureName[BMFONT_MAX_TEXTURE_NAME_LENGTH];
};

struct BMFontBlockCommon
{
public:
    unsigned short mLineHeight;
    unsigned short mBase;
    unsigned short mScaleW;
    unsigned short mScaleH;
    unsigned short mPages;
    unsigned char mBitField;
    unsigned char mAlphaChannel;
    unsigned char mRedChannel;
    unsigned char mGreenChannel;
    unsigned char mBlueChannel;
};

struct BMFontBlockChar
{
public:
    unsigned int mID;
    unsigned short mX;
    unsigned short mY;
    unsigned short mW;
    unsigned short mH;
    signed short mOffsetX;
    signed short mOffsetY;
    signed short mAdvanceX;
    unsigned char mPage;
    unsigned char mChannel;
};

struct BMFontBlockKerningPairs
{
public:
    unsigned int mFirst;
    unsigned int mSecond;
    signed short mAmount;
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////

struct BMFontMetadata
{
public:
    BMFontBlockInfo mInfo;
    BMFontBlockInfoEx mInfoEx;
    BMFontBlockCommon mCommon;
    BMFontBlockChar mChars[BMFONT_NUMCHARS];
};

//////////////////////////////////////////////////////////////////////////

enum class BMFontError 
{
    Success,
    ReadFailed,
    InvalidFormat,
    InvalidVersion,
    DataCorrupted,
    UnsupportedFeature
};

inline const char* ToString(BMFontError errorcode)
{
    switch (errorcode)
    {
        case BMFontError::Success: return "success";
        case BMFontError::ReadFailed: return "read_failed";
        case BMFontError::InvalidFormat: return "invalid_format";
        case BMFontError::InvalidVersion: return "invalid_version";
        case BMFontError::DataCorrupted: return "data_corrupted";
        case BMFontError::UnsupportedFeature: return "unsupported_feature";
    }
    cxx_assert(false);
    return "";
}

//////////////////////////////////////////////////////////////////////////

// Read bmfont medata from stream
// @param bytestream: Source data stream
// @param metadata: Output metadata
// @param errorcode: Error code
bool BMFontReadMetadata(std::istream& bytestream, BMFontMetadata& metadata, BMFontError& errorcode);
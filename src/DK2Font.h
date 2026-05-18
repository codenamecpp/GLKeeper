#pragma once

//////////////////////////////////////////////////////////////////////////

struct DK2FontEntry
{
public:
    unsigned short mCharacter = 0;
    int mWidth = 0;
    int mHeight = 0;
    int mOffsetX = 0;
    int mOffsetY = 0;
    int mOuterWidth = 0;
    std::vector<unsigned char> mImage; // image format single gray channel
};

//////////////////////////////////////////////////////////////////////////

using DK2FontEntries = std::vector<DK2FontEntry>;

//////////////////////////////////////////////////////////////////////////

struct DK2FontDesc
{
public:
    unsigned short mMaxCodePoint = 0;
    int mMaxWidth = 0;
    int mMaxHeight = 0;
    int mAvgWidth = 0;
    DK2FontEntries mEntries;
};

//////////////////////////////////////////////////////////////////////////

bool DK2_BF4_LoadFromStream(std::istream& theInputStream, DK2FontDesc& theFont);

//////////////////////////////////////////////////////////////////////////
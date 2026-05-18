#include "stdafx.h"
#include "DK2Text.h"

//////////////////////////////////////////////////////////////////////////

#define DK2_BFMU_FILE_HEADER_ID 0x554d4642
#define DK2_BFST_FILE_HEADER_ID 0x54534642

//////////////////////////////////////////////////////////////////////////

bool DK2MultiByteToUnicode::LoadFromFile(const std::string& filePath)
{
    Clear();

    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);
    if (!fileStream)
        return false;
    
    uint32_t header;
    if (!cxx::read_elements(fileStream, &header, 1))
    {
        cxx_assert(false);
        return false;
    }

    if (header != DK2_BFMU_FILE_HEADER_ID)
    {
        cxx_assert(false);
        return false;
    }

    unsigned char singleCount = 0;
    unsigned char unknownByte = 0;
    unsigned char mbHiBound = 0;
    unsigned char mbLoBound = 0;

    if (!cxx::read_elements(fileStream, &singleCount, 1) ||
        !cxx::read_elements(fileStream, &unknownByte, 1) ||
        !cxx::read_elements(fileStream, &mbHiBound, 1) ||
        !cxx::read_elements(fileStream, &mbLoBound, 1))
    {
        cxx_assert(false);
        return false;
    }

    mUnicodeChars.reserve(512);
    // unicode characters
    while (fileStream)
    {
        wchar_t unicodeChar = 0;
        if (!cxx::read_elements(fileStream, &unicodeChar, 1))
            break;

        mUnicodeChars.push_back(unicodeChar);
    }

    bool isSuccess = IsLoaded();
    if (isSuccess)
    {
        mSingleByteCharsCount = singleCount;
        mMultiByteHiBound = mbHiBound;
        mMultiByteLoBound = mbLoBound;
        mThreshold = (255 - mSingleByteCharsCount);
    }
    return isSuccess;
}

void DK2MultiByteToUnicode::Clear()
{
    mSingleByteCharsCount = 0;
    mMultiByteHiBound = 0;
    mMultiByteLoBound = 0;
    mThreshold = 0;
    mUnicodeChars.clear();
}

bool DK2MultiByteToUnicode::DecodeMultiByteString(const DK2MultiByteString& encoded, std::wstring& resultString) const
{
    bool isSuccess = IsLoaded();
    if (isSuccess)
    {
        resultString.clear();
        if (!encoded.empty())
        {
            auto src_chars_it = encoded.begin();
            for (; src_chars_it != encoded.end(); )
            {
                wchar_t index = 0;

                unsigned char mbb = *(src_chars_it++) - 1;
                // single byte character?
                if (mbb < mThreshold)
                {
                    index = mbb;
                }
                // multi byte character
                else
                {
                    if (src_chars_it == encoded.end())
                    {
                        cxx_assert(false);
                        break;
                    }

                    index = ((mbb * 255) - mThreshold * 254 + *src_chars_it++);
                }

                wchar_t c = (index < mUnicodeChars.size()) ? mUnicodeChars[index] : 0;
                resultString.push_back(c);
            }
        }
    }
    return isSuccess;
}

bool DK2MultiByteToUnicode::IsLoaded() const
{
    return !mUnicodeChars.empty();
}

//////////////////////////////////////////////////////////////////////////

bool DK2EncodedStrings::LoadFromFile(const std::string& filePath)
{
    Clear();

    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);
    if (!fileStream)
        return false;

    uint32_t header;
    if (!cxx::read_elements(fileStream, &header, 1))
    {
        cxx_assert(false);
        return false;
    }

    if (header != DK2_BFST_FILE_HEADER_ID)
    {
        cxx_assert(false);
        return false;
    }

    unsigned int key = 0;
    unsigned int stringsCount = 0;
    if (!cxx::read_elements(fileStream, &key, 1) ||
        !cxx::read_elements(fileStream, &stringsCount, 1))
    {
        cxx_assert(false);
        return false;
    }

    unsigned int stringOffsetsBaseOffset = static_cast<unsigned int>(fileStream.tellg());

    mOffsets.clear();
    mOffsets.resize(stringsCount);

    if (!cxx::read_elements(fileStream, mOffsets.data(), stringsCount))
    {
        cxx_assert(false);
        return false;
    }

    mStringsList.resize(stringsCount);

    bool isSuccess = true;
    for (unsigned int stringIndex = 0; stringIndex < stringsCount; ++stringIndex)
    {
        unsigned int dataOffset = stringOffsetsBaseOffset + mOffsets[stringIndex];

        isSuccess = !!fileStream.seekg(dataOffset, std::ios::beg);
        cxx_assert(isSuccess);
        if (!isSuccess) break;

        unsigned int header = 0;
        isSuccess = cxx::read_elements(fileStream, &header, 1);
        cxx_assert(isSuccess);
        if (!isSuccess) break;

        unsigned char flags = header & 0x000000FF;
        if (flags == 1)
        {
            unsigned int charactersCount = (header & 0xFFFFFF00) >> 8;
            cxx_assert(charactersCount < 2048);
            mStringsList[stringIndex].resize(charactersCount);

            isSuccess = cxx::read_elements(fileStream, mStringsList[stringIndex].data(), charactersCount);
            cxx_assert(isSuccess);
            if (!isSuccess) break;
        }
        unsigned int nul = 0;
        if (!cxx::read_elements(fileStream, &nul, 1))
            break;
    }

    if (!isSuccess)
    {
        mOffsets.clear();
        mStringsList.clear();
    }
    mStringsListKeyFromIndex = key;
    return isSuccess;
}

void DK2EncodedStrings::Clear()
{
    mStringsListKeyFromIndex = 0;
    mStringsList.clear();
    mOffsets.clear();
}

bool DK2EncodedStrings::IsLoaded()
{
    return !mStringsList.empty();
}

const DK2MultiByteString& DK2EncodedStrings::GetString(int stringIndex) const
{
    static const DK2MultiByteString nullString;
    if (stringIndex < GetStringCount())
    {
        return mStringsList[stringIndex];
    }
    cxx_assert(false);
    return nullString;
}

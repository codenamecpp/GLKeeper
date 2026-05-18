#pragma once

//////////////////////////////////////////////////////////////////////////

using DK2MultiByteString = std::vector<uint8_t>;

//////////////////////////////////////////////////////////////////////////

class DK2MultiByteToUnicode
{
public:
    DK2MultiByteToUnicode() = default;

    bool LoadFromFile(const std::string& filePath);
    void Clear();

    bool IsLoaded() const;

    bool DecodeMultiByteString(const DK2MultiByteString& encoded, std::wstring& resultString) const;

private:
    unsigned char mSingleByteCharsCount = 0;
    unsigned char mMultiByteHiBound = 0; // 0x06
    unsigned char mMultiByteLoBound = 0; // 0x07
    unsigned char mThreshold = 0;
    std::vector<wchar_t> mUnicodeChars;
};

//////////////////////////////////////////////////////////////////////////

class DK2EncodedStrings
{
public:
    DK2EncodedStrings() = default;

    bool LoadFromFile(const std::string& filePath);
    void Clear();

    bool IsLoaded();

    const DK2MultiByteString& GetString(int stringIndex) const;
    inline int GetStringCount() const 
    {
        return static_cast<int>(mStringsList.size());
    }
    inline int GetStringsListKeyFromIndex() const { return mStringsListKeyFromIndex;
    }
private:
    unsigned int mStringsListKeyFromIndex = 0;
    std::vector<unsigned int> mOffsets;
    std::vector<DK2MultiByteString> mStringsList;
};

//////////////////////////////////////////////////////////////////////////
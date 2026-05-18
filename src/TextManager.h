#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "DK2Text.h"

//////////////////////////////////////////////////////////////////////////

class TextManager: public cxx::noncopyable
{
private:

    //////////////////////////////////////////////////////////////////////////
    using TextTable = std::vector<std::wstring>;
    using TextTableMap = std::unordered_map<TextTableId, TextTable>;
    //////////////////////////////////////////////////////////////////////////

public:
    bool Initialize();
    void Shutdown();

    inline std::wstring_view GetString(TextTableId textTableId, int stringIndex) const
    {
        auto table_it = mTextTableMap.find(textTableId);
        if (table_it != mTextTableMap.end())
        {
            const TextTable& textTable = table_it->second;
            if ((stringIndex > 0) && (stringIndex < static_cast<int>(textTable.size())))
                return textTable[stringIndex];
        }
        return {};
    }
    bool HasTextTable(TextTableId textTableId) const;

private:
    bool LoadTextTable(const std::string& filePath);

private:
    TextTableMap mTextTableMap;
    DK2EncodedStrings mTempEncodedStringsBuffer;
    DK2MultiByteToUnicode mMBToUnicode;
};

//////////////////////////////////////////////////////////////////////////

extern TextManager gTexts;

//////////////////////////////////////////////////////////////////////////
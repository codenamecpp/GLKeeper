#include "stdafx.h"
#include "TextManager.h"

//////////////////////////////////////////////////////////////////////////

TextManager gTexts;

//////////////////////////////////////////////////////////////////////////

bool TextManager::Initialize()
{


    std::string mbToUnicodeFilePath;
    if (!gFiles.LocateTextTableFile("MBToUni.dat", mbToUnicodeFilePath))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot locate multi-byte-to-unicode conversion table file");
        return false;
    }

    if (!mMBToUnicode.LoadFromFile(mbToUnicodeFilePath))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load multi-byte-to-unicode conversion table");
        return false;
    }

    gFiles.EnumTextTableFiles([this](const std::string& fileName)
        {
            std::string filePath;
            if (gFiles.LocateTextTableFile(fileName, filePath))
            {
                if (!this->LoadTextTable(filePath))
                {
                    gConsole.LogMessage(eLogLevel_Warning, "Cannot load text table '%s'", fileName.c_str());
                }
            }
            else
            {
                cxx_assert(false);
            }
        });

    if (!HasTextTable(TextTableId_Main) || !HasTextTable(TextTableId_Speech))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Missing primary text tables");
        cxx_assert(false);
    }

    return true;
}

void TextManager::Shutdown()
{
    mTextTableMap.clear();
    mTempEncodedStringsBuffer.Clear();
    mMBToUnicode.Clear();
}

bool TextManager::HasTextTable(TextTableId textTableId) const
{
    cxx_assert(textTableId != TextTableId_Null);

    return mTextTableMap.find(textTableId) != mTextTableMap.end();
}

bool TextManager::LoadTextTable(const std::string& filePath)
{
    if (!mTempEncodedStringsBuffer.LoadFromFile(filePath))
    {
        cxx_assert(false);
        return false;
    }

    TextTableId tableId = mTempEncodedStringsBuffer.GetStringsListKeyFromIndex();
    if (HasTextTable(tableId))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Text Table '%d' already exists", tableId);

        cxx_assert(false);
        return true;
    }

    const int NumStrings = mTempEncodedStringsBuffer.GetStringCount();

    TextTable& textTable = mTextTableMap[tableId];
    textTable.resize(NumStrings);

    for (int istring = 0; istring < NumStrings; ++istring)
    {
        if (!mMBToUnicode.DecodeMultiByteString(mTempEncodedStringsBuffer.GetString(istring), textTable[istring]))
        {
            cxx_assert(false);
        }
    }
    mTempEncodedStringsBuffer.Clear();
    return true;
}

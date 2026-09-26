#include "stdafx.h"
#include "DK2SfxBankFile.h"

DK2SfxBankFile::DK2SfxBankFile()
{

}

bool DK2SfxBankFile::OpenFile(const std::string& filePath)
{
    CloseFile();

    std::ifstream fileStream {filePath, std::ios::in | std::ios::binary};
    if (!fileStream)
        return false;

    unsigned int headerA = cxx::read_int32(fileStream);
    unsigned int headerB = cxx::read_int32(fileStream);
    unsigned int headerC = cxx::read_int32(fileStream);
    unsigned int headerD = cxx::read_int32(fileStream);

    bool hasHeaders = 
        (headerA == 0xE9612C01U) &&
        (headerB == 0x11D231D0U) &&
        (headerC == 0xA00009B4U) &&
        (headerD == 0x03F293C9U);

    cxx_assert(hasHeaders);

    if (!hasHeaders)
    {
        CloseFile();
        return false;
    }

    const unsigned int unknown1 = cxx::read_int32(fileStream);
    const unsigned int unknown2 = cxx::read_int32(fileStream);

    const unsigned int entriesCount = cxx::read_int32(fileStream);

    mEntries.resize(entriesCount);
    for (BankEntry& roller: mEntries)
    {
        const unsigned int unknown1 = cxx::read_int32(fileStream);
        const unsigned int unknown2 = cxx::read_int32(fileStream);
        const unsigned int unknown3 = cxx::read_int16(fileStream);
        const unsigned int unknown4 = cxx::read_int8(fileStream);

        if (!fileStream)
        {
            cxx_assert(false);

            CloseFile();
            return false;
        }
    }

    for (BankEntry& roller: mEntries)
    {
        unsigned int stringLength = cxx::read_int32(fileStream);
        if (!cxx::read_fixed_ansi_cstring(fileStream, stringLength, roller.mArchiveName))
        {
            cxx_assert(false);

            CloseFile();
            return false;
        }
    }

    return true;
}

void DK2SfxBankFile::CloseFile()
{
    mEntries.clear();
}

bool DK2SfxBankFile::IsOpened() const
{
    return !mEntries.empty();
}
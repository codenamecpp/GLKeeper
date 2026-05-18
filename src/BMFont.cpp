#include "stdafx.h"
#include "BMFont.h"

bool BMFontReadMetadata(std::istream& bytestream, BMFontMetadata& metadata, BMFontError& errorcode)
{
    errorcode = BMFontError::Success;

    // read header
    unsigned long header = 0;

    if (!cxx::read_elements(bytestream, &header, 1))
    {
        errorcode = BMFontError::ReadFailed;
        return false;
    }

    // verify magic number
    if (header != BMFONT_SIGNATURE)
    {
        errorcode = BMFontError::InvalidFormat;
        return false;
    }

    // read sections
    for(;;) 
    {
        BMFontBlockHeader blockHeader;
        if (!cxx::read_elements(bytestream, &blockHeader, 1))
            break;

        std::streampos currentStreamPos = bytestream.tellg();
        if (blockHeader.mID == BMFONT_BLOCKID_INFO)
        {
            if (!cxx::read_elements(bytestream, &metadata.mInfo, 1))
            {
                errorcode = BMFontError::DataCorrupted;
                return false;        
            }

            // unicode in not supported
            if ((metadata.mInfo.mBits & 0x02) == 0x02)
            {
                errorcode = BMFontError::UnsupportedFeature;
                return false;
            }

            // read font name
            int fontNameLength = blockHeader.mLength - sizeof(metadata.mInfo);
            if (fontNameLength > BMFONT_MAX_FONT_NAME_LENGTH)
            {
                errorcode = BMFontError::DataCorrupted;
                return false;
            }

            if (!bytestream.read(metadata.mInfoEx.mFontName, fontNameLength))
            {
                errorcode = BMFontError::DataCorrupted;
                return false;
            }
        }
        else if (blockHeader.mID == BMFONT_BLOCKID_COMMON)
        {
            if (!cxx::read_elements(bytestream, &metadata.mCommon, 1))
            {
                errorcode = BMFontError::DataCorrupted;
                return false;
            }
        }
        else if (blockHeader.mID == BMFONT_BLOCKID_CHARS)
        {
            unsigned numCharacters = blockHeader.mLength / sizeof(BMFontBlockChar);
            for (unsigned i = 0; i < numCharacters; ++i) // read characters
            {
                BMFontBlockChar _char = {};

                if (!cxx::read_elements(bytestream, &_char, 1))
                {
                    errorcode = BMFontError::DataCorrupted;
                    return false;
                }

                // map character
                if (_char.mID < _countof(metadata.mChars))
                {
                    metadata.mChars[_char.mID] = _char;
                }
            }
        }
        else if (blockHeader.mID == BMFONT_BLOCKID_PAGES) // texture names
        {
            if (metadata.mCommon.mPages > 1 || metadata.mCommon.mPages < 1) // single page (texture) supported only
            {
                errorcode = BMFontError::UnsupportedFeature;
                return false;
            }

            if (blockHeader.mLength > BMFONT_MAX_TEXTURE_NAME_LENGTH)
            {
                errorcode = BMFontError::DataCorrupted;
                return false;
            }

            if (!bytestream.read(metadata.mInfoEx.mTextureName, blockHeader.mLength))
            {
                errorcode = BMFontError::DataCorrupted;
                return false;
            }
        }

        std::streampos expectedPosition = currentStreamPos + std::streamoff(blockHeader.mLength);

        currentStreamPos = bytestream.tellg();
        if (currentStreamPos != expectedPosition)
        {
            bytestream.seekg(expectedPosition, std::ios::beg);

            currentStreamPos = bytestream.tellg();
            if (currentStreamPos != expectedPosition)
            {
                errorcode = BMFontError::DataCorrupted;
                return false;
            }
        }
    }
    return true;
}
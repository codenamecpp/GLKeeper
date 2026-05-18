#include "stdafx.h"
#include "Font.h"
#include "BMFont.h"
#include "DK2Font.h"

//////////////////////////////////////////////////////////////////////////

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#include "stb_rect_pack.h"

//////////////////////////////////////////////////////////////////////////

Font::Font(const std::string& fontName) 
    : mFontName(fontName)
{
}

void Font::Load()
{
    Purge();
    
    if (LoadFont_FromFile())
    {
        mIsLoadedFromFile = true;
    }
    else
    {
        InitDefault();
    }

    mTexturePixelFormat = mAtlasBitmap.GetPixelFormat();
}

void Font::InitDefault()
{
    Purge();

    mIsDefaultFont = true;

    mLineHeight = 10;
    mAtlasBitmap.Create(ePixelFormat_R8, Point2D{8, 8}, COLOR_WHITE);

    mWideChars.clear();
    for (CharEntry& roller: mAnsiChars)
    {
        roller.mRect.x = 0;
        roller.mRect.y = 0;
        roller.mRect.w = 8;
        roller.mRect.h = 8;
    }
    InitTexcoords();

    mTexturePixelFormat = mAtlasBitmap.GetPixelFormat();
}

void Font::Purge()
{
    mIsRenderDataInited = false;
    mIsDefaultFont = false;
    mIsLoadedFromFile = false;
    mLineHeight = 0;
    mTexturePixelFormat = ePixelFormat_Null;
    mAtlasBitmap.Clear();
    mWideChars.clear();
    mGpuTextureResource.reset();
}

void Font::InitRenderData()
{
    if (IsRenderDataInited())
        return;

    if (!IsInited())
    {
        cxx_assert(false);
        return;
    }

    mIsRenderDataInited = true;

    static bool enableFiltering = true;
    static bool freeSystemBits = true;

    if (mAtlasBitmap.HasContent())
    {
        eTextureFiltering texFilteting = enableFiltering ? eTextureFiltering_Bilinear : eTextureFiltering_None;

        // create texture
        mGpuTextureResource = gRenderDevice.CreateTexture2D(mAtlasBitmap, texFilteting, eTextureRepeating_ClampToEdge);
        cxx_assert(mGpuTextureResource);
        if (mGpuTextureResource)
        {
            // success
            if (freeSystemBits)
            {
                mAtlasBitmap.Clear();
            }

            return;
        }
    }

    cxx_assert(false);

    gConsole.LogMessage(eLogLevel_Warning, "Cannot initialize font render data ('%s'), trying fallback", mFontName.c_str());

    // fallback in case of failure
    BitmapImage bi;
    if (!bi.Create(ePixelFormat_RGBA8, Point2D { 8, 8 }, COLOR_WHITE))
    {
        cxx_assert(false);
    }

    mGpuTextureResource = gRenderDevice.CreateTexture2D(bi);
    cxx_assert(!!mGpuTextureResource);
}

void Font::BuildTextMesh(const std::wstring_view& wideString, const Point2D& pos, Color32 color, std::vector<Quad2D>& outQuads) const
{
    outQuads.clear();

    const int printablesCount = CountPrintableCharacters(wideString);
    if (printablesCount == 0)
        return;

    outQuads.reserve(printablesCount);

    // generate vertices
    int currX = pos.x;
    int currY = pos.y;
    for (wchar_t wideChar: wideString)
    {
        if (wideChar == L'\n')
        {
            currX = pos.x;
            currY += mLineHeight;
            continue;
        }

        if (wideChar < L' ') continue;

        const CharEntry& fontchar = GetCharEntry(wideChar);

        int spacew = (fontchar.mRect.w == 0) ? fontchar.mOuterWidth : fontchar.mRect.w;
        if (spacew == 0)
            continue;

        if (wideChar == L' ')
        {
            currX += spacew;
            continue;
        }

        Quad2D& characterQuad = outQuads.emplace_back();

        // setup quad vertices in specific order
        characterQuad.mPoints[0].mColor         = color;
        characterQuad.mPoints[0].mTexcoord[0]   = fontchar.mTexcoords.mU0;
        characterQuad.mPoints[0].mTexcoord[1]   = fontchar.mTexcoords.mV0;
        characterQuad.mPoints[0].mPosition.x    = (currX + fontchar.mOffset.x) * 1.0f;
        characterQuad.mPoints[0].mPosition.y    = (currY + fontchar.mOffset.y) * 1.0f;

        characterQuad.mPoints[1].mColor         = color;
        characterQuad.mPoints[1].mTexcoord[0]   = fontchar.mTexcoords.mU0;
        characterQuad.mPoints[1].mTexcoord[1]   = fontchar.mTexcoords.mV1;
        characterQuad.mPoints[1].mPosition.x    = (currX + fontchar.mOffset.x) * 1.0f;
        characterQuad.mPoints[1].mPosition.y    = (currY + fontchar.mRect.h + fontchar.mOffset.y) * 1.0f;

        characterQuad.mPoints[2].mColor         = color;
        characterQuad.mPoints[2].mTexcoord[0]   = fontchar.mTexcoords.mU1;
        characterQuad.mPoints[2].mTexcoord[1]   = fontchar.mTexcoords.mV1;
        characterQuad.mPoints[2].mPosition.x    = (currX + fontchar.mOffset.x + spacew) * 1.0f;
        characterQuad.mPoints[2].mPosition.y    = (currY + fontchar.mRect.h + fontchar.mOffset.y) * 1.0f;

        characterQuad.mPoints[3].mColor         = color;
        characterQuad.mPoints[3].mTexcoord[0]   = fontchar.mTexcoords.mU1;
        characterQuad.mPoints[3].mTexcoord[1]   = fontchar.mTexcoords.mV0;
        characterQuad.mPoints[3].mPosition.x    = (currX + fontchar.mOffset.x + spacew) * 1.0f;
        characterQuad.mPoints[3].mPosition.y    = (currY + fontchar.mOffset.y) * 1.0f;

        currX += fontchar.mOuterWidth;
    }
}

void Font::BuildTextMesh(const std::wstring_view& wideString, const Rect2D& rect, 
    eTextHorzAlignment horzAlign, 
    eTextVertAlignment vertAlign, Color32 color, std::vector<Quad2D>& outQuads) const
{
    outQuads.clear();

    bool hasHorzBounds = (rect.w > 0);
    bool hasVertBounds = (rect.h > 0);

    const int printablesCount = CountPrintableCharacters(wideString);
    if (printablesCount == 0)
        return;

    outQuads.reserve(printablesCount);

    // tokenize text

    struct LineWord
    {
        std::wstring_view::const_iterator begin_it;
        std::wstring_view::const_iterator end_it;
        int wordWidth = 0;
    };

    struct LineEntry
    {
        Temp_List<LineWord> wordsList;
        int lineWidth = 0;
    };

    Temp_List<LineEntry> linesList;
    linesList.emplace_back();

    for (auto curr_char_it = wideString.begin(), string_end_it = wideString.end(); 
        curr_char_it != string_end_it; )
    {
        // get next word
        LineWord currWord;
        currWord.begin_it = curr_char_it;
        currWord.end_it = std::find_if(curr_char_it + 1, string_end_it, [](wchar_t wideChar){ return wideChar <= L' '; });
        for (auto word_it = currWord.begin_it; word_it != currWord.end_it; ++word_it)
        {
            const CharEntry& fontchar = GetCharEntry(*word_it);
            int spacew = (fontchar.mRect.w == 0) ? fontchar.mOuterWidth : fontchar.mRect.w;
            if (spacew > 0)
            {
                currWord.wordWidth += fontchar.mOuterWidth;
            }
        }

        if (currWord.wordWidth > 0)
        {
            // word wrap detection logic
            if (hasHorzBounds)
            {
                int currWidth = linesList.back().lineWidth;
                if ((currWidth > 0) && ((currWidth + currWord.wordWidth) > rect.w))
                {
                    // line overflow, move to next line
                    LineEntry& nextLine = linesList.emplace_back();
                }
            }
            // append word
            LineEntry& currLine = linesList.back();
            currLine.lineWidth += currWord.wordWidth;
            currLine.wordsList.push_back(currWord);
        }
        
        curr_char_it = currWord.end_it;
        if ((curr_char_it == string_end_it) || (*curr_char_it == 0)) break;
        if (*curr_char_it == L'\n')
        {
            // start new line
            linesList.emplace_back();
        }
    }

    int currY = rect.y;

    for (const LineEntry& currLine: linesList)
    {
        int currX = rect.x;
        if (hasHorzBounds && (currLine.lineWidth <= rect.w))
        {
            switch (horzAlign)
            {
                case eTextHorzAlignment_Center:
                    currX = (rect.w / 2) - currLine.lineWidth / 2;
                break;
                case eTextHorzAlignment_Right:
                    currX = (rect.x + rect.w - 1) - currLine.lineWidth;
                break;
            }
        }
        for (const LineWord& currWord: currLine.wordsList)
        {
            for (auto it = currWord.begin_it; it != currWord.end_it; ++it)
            {
                wchar_t wideChar = *it;

                if (wideChar < L' ') continue;

                const CharEntry& fontchar = GetCharEntry(wideChar);

                int spacew = (fontchar.mRect.w == 0) ? fontchar.mOuterWidth : fontchar.mRect.w;
                if (spacew == 0)
                    continue;

                if (wideChar == L' ')
                {
                    currX += spacew;
                    continue;
                }

                Quad2D& characterQuad = outQuads.emplace_back();

                // setup quad vertices in specific order
                characterQuad.mPoints[0].mColor         = color;
                characterQuad.mPoints[0].mTexcoord[0]   = fontchar.mTexcoords.mU0;
                characterQuad.mPoints[0].mTexcoord[1]   = fontchar.mTexcoords.mV0;
                characterQuad.mPoints[0].mPosition.x    = (currX + fontchar.mOffset.x) * 1.0f;
                characterQuad.mPoints[0].mPosition.y    = (currY + fontchar.mOffset.y) * 1.0f;

                characterQuad.mPoints[1].mColor         = color;
                characterQuad.mPoints[1].mTexcoord[0]   = fontchar.mTexcoords.mU0;
                characterQuad.mPoints[1].mTexcoord[1]   = fontchar.mTexcoords.mV1;
                characterQuad.mPoints[1].mPosition.x    = (currX + fontchar.mOffset.x) * 1.0f;
                characterQuad.mPoints[1].mPosition.y    = (currY + fontchar.mRect.h + fontchar.mOffset.y) * 1.0f;

                characterQuad.mPoints[2].mColor         = color;
                characterQuad.mPoints[2].mTexcoord[0]   = fontchar.mTexcoords.mU1;
                characterQuad.mPoints[2].mTexcoord[1]   = fontchar.mTexcoords.mV1;
                characterQuad.mPoints[2].mPosition.x    = (currX + fontchar.mOffset.x + spacew) * 1.0f;
                characterQuad.mPoints[2].mPosition.y    = (currY + fontchar.mRect.h + fontchar.mOffset.y) * 1.0f;

                characterQuad.mPoints[3].mColor         = color;
                characterQuad.mPoints[3].mTexcoord[0]   = fontchar.mTexcoords.mU1;
                characterQuad.mPoints[3].mTexcoord[1]   = fontchar.mTexcoords.mV0;
                characterQuad.mPoints[3].mPosition.x    = (currX + fontchar.mOffset.x + spacew) * 1.0f;
                characterQuad.mPoints[3].mPosition.y    = (currY + fontchar.mOffset.y) * 1.0f;

                currX += fontchar.mOuterWidth;
            }
        }
        currY += mLineHeight;
    }
}

Point2D Font::ComputeTextDims(const std::wstring_view& wideString) const
{
    Point2D resultDims { 0, 0 };

    for (wchar_t wideChar: wideString)
    {
        if (wideChar == 0) break;
        if (wideChar == L'\n')
        {
            resultDims.x = 0;
            resultDims.y += mLineHeight;
            continue;
        }

        if (wideChar < L' ') continue;

        const CharEntry& charEntry = GetCharEntry(wideChar);
        int spacew = (charEntry.mRect.w == 0) ? charEntry.mOuterWidth : charEntry.mRect.w;
        if (spacew == 0)
            continue;

        if (wideChar == L' ')
        {
            resultDims.x += spacew;
            continue;
        }
        resultDims.x += charEntry.mOuterWidth;
    }
    return resultDims;
}

int Font::CountPrintableCharacters(const std::wstring_view& wideString) const
{
    int printablesCount = 0;
    for (wchar_t wideChar: wideString)
    {
        if (wideChar == 0) break;
        if (wideChar <= L' ')
            continue;

        const CharEntry& charEntry = GetCharEntry(wideChar);
        int spacew = (charEntry.mRect.w == 0) ? charEntry.mOuterWidth : charEntry.mRect.w;
        if (spacew == 0)
            continue;

        ++printablesCount;
    }
    return printablesCount;
}

bool Font::LoadFont_FromFile()
{
    bool bmfont = false;

    const std::string& fontName = mFontName;

    if (fontName.empty())
    {
        cxx_assert(false);
        return false;
    }

    std::string fontFilePath;
    if (!gFiles.LocateFont(fontName + ".bf4", fontFilePath))
    {
        if (gFiles.LocateFont(fontName + ".fnt", fontFilePath))
        {
            bmfont = true;
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot locate font file '%s'", fontName.c_str());
            return false;
        }
    }

    std::ifstream fileStream(fontFilePath, std::ios::in | std::ios::binary);
    if (!fileStream.is_open())
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot open font file '%s'", fontName.c_str());
        return false;
    }

    bool isSuccess = bmfont ? 
        LoadFont_BMF(fileStream) : 
        LoadFont_BF4(fileStream);

    if (isSuccess)
    {
        InitTexcoords();
    }
    else
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load font data '%s'", fontName.c_str());
    }
    return isSuccess;
}

bool Font::LoadFont_BF4(std::istream& bitstream)
{
    DK2FontDesc bf4Font;

    if (!DK2_BF4_LoadFromStream(bitstream, bf4Font))
    {
        gConsole.LogMessage(eLogLevel_Warning, "BF4 font reading failed");
        return false;
    }

    mLineHeight = bf4Font.mMaxHeight;

    // setup glyphs
    int numCharacters = 0;
    for (const DK2FontEntry& fontChracter: bf4Font.mEntries)
    {
        CharEntry& charEntry = (fontChracter.mCharacter < AnsiCharsCount) ?
            mAnsiChars[fontChracter.mCharacter] : 
            mWideChars[fontChracter.mCharacter];

        charEntry.mRect.h = fontChracter.mHeight;
        charEntry.mRect.w = fontChracter.mWidth;
        charEntry.mOffset.x = fontChracter.mOffsetX;
        charEntry.mOffset.y = fontChracter.mOffsetY;
        charEntry.mOuterWidth = fontChracter.mOuterWidth;
        ++numCharacters; // num specified characters
    }

    std::vector<stbrp_node> stbrp_nodes(MaxAtlasSize);
    std::vector<stbrp_rect> stbrp_rects(numCharacters);

    // prepare characters
    for (int ichar = 0, icurr = 0, MAX = bf4Font.mEntries.size(); ichar < MAX; ++ichar)
    {
        if (bf4Font.mEntries[ichar].mHeight > 0 && bf4Font.mEntries[ichar].mWidth > 0)
        {
            stbrp_rects[icurr].id = ichar;
            stbrp_rects[icurr].w = bf4Font.mEntries[ichar].mWidth + 2;
            stbrp_rects[icurr].h = bf4Font.mEntries[ichar].mHeight + 2;
            ++icurr;
        }
    }

    int currentAtlasSizeX = 128;
    int currentAtlasSizeY = 128;
    int nn = 0;

    for (bool isPacked = false; !isPacked; ++nn)
    {
        // reset status
        for (stbrp_rect& rc: stbrp_rects)
        {
            rc.was_packed = 0;
        }

        stbrp_context context;
        stbrp_init_target(&context, currentAtlasSizeX, currentAtlasSizeY, stbrp_nodes.data(), stbrp_nodes.size());
        isPacked = stbrp_pack_rects(&context, stbrp_rects.data(), stbrp_rects.size()) > 0;
        if (!isPacked)
        {
            if (nn & 1) { currentAtlasSizeY <<= 1; } // increase size
            else { currentAtlasSizeX <<= 1; } // increase size

            if ((currentAtlasSizeX > MaxAtlasSize) || 
                (currentAtlasSizeY > MaxAtlasSize)) 
            {
                gConsole.LogMessage(eLogLevel_Warning, "BF4 font atlas generation exceeds size limit");
                return false;
            }
        }
    }

    for (const stbrp_rect& rc: stbrp_rects)
    {
        wchar_t char_index = bf4Font.mEntries[rc.id].mCharacter;

        CharEntry& charEntry = (char_index < AnsiCharsCount) ? mAnsiChars[char_index] : mWideChars[char_index];
        charEntry.mRect.x = rc.x;
        charEntry.mRect.y = rc.y;
    }

    // generate atlas

    Point2D atlasDims { currentAtlasSizeX, currentAtlasSizeY };
    if (!mAtlasBitmap.Create(ePixelFormat_R8, atlasDims, nullptr))
    {
        gConsole.LogMessage(eLogLevel_Warning, "BM4 font atlas pixels allocation failed");
        return nullptr;
    }

    mAtlasBitmap.SetHasAlphaHint(true);

    unsigned char* sourceImagePixels = mAtlasBitmap.GetMipPixels(0);
    cxx_assert(sourceImagePixels);

    for (int iy = 0; iy < currentAtlasSizeY; ++iy)
    {
        for (int ix = 0; ix < currentAtlasSizeX; ++ix)
        {
            sourceImagePixels[iy * currentAtlasSizeX + ix] = 0;
        }
    }

    for (stbrp_rect& rc: stbrp_rects)
    {
        unsigned char* char_pixels = bf4Font.mEntries[rc.id].mImage.data();
        // copy pixels
        const int char_real_w = bf4Font.mEntries[rc.id].mWidth;
        const int char_real_h = bf4Font.mEntries[rc.id].mHeight;
        for (int iy = 0; iy < char_real_h; ++iy)
        {
            for (int ix = 0; ix < char_real_w; ++ix)
            {
                sourceImagePixels[(iy + rc.y) * currentAtlasSizeX + (ix + rc.x)] = char_pixels[iy * char_real_w + ix];
            }
        }
    }

    return true;
}

bool Font::LoadFont_BMF(std::istream& bitstream)
{
    BMFontMetadata metadata;
    BMFontError errorcode;
    if (!BMFontReadMetadata(bitstream, metadata, errorcode))
    {
        gConsole.LogMessage(eLogLevel_Warning, "BMFont metadata reading failed, error '%s'", ToString(errorcode));
        return false;
    }

    std::string textureName = cxx::va("fonts/%s", metadata.mInfoEx.mTextureName);
    std::string resourcePath;

    if (gFiles.PathToFile(textureName, resourcePath))
    {
        if (!mAtlasBitmap.LoadFromFile(resourcePath))
        {
            gConsole.LogMessage(eLogLevel_Warning, "BMFont atlas loading error, '%s'", textureName.c_str());
            return false;
        }
    }
    else
    {
        gConsole.LogMessage(eLogLevel_Warning, "BMFont atlas missing '%s'", textureName.c_str());
        return false;
    }

    const int CharactersCount = std::min<int>(AnsiCharsCount, BMFONT_NUMCHARS);
    for (int ichar = 0; ichar < CharactersCount; ++ichar)
    {
        BMFontBlockChar& srcChar = metadata.mChars[ichar];
        CharEntry& dstChar = mAnsiChars[ichar];
        dstChar.mOffset.x = srcChar.mOffsetX;
        dstChar.mOffset.y = srcChar.mOffsetY;
        dstChar.mRect.x = srcChar.mX;
        dstChar.mRect.y = srcChar.mY;
        dstChar.mRect.w = srcChar.mW;
        dstChar.mRect.h = srcChar.mH;
        dstChar.mOuterWidth = srcChar.mAdvanceX;
    }
    mLineHeight = metadata.mCommon.mLineHeight;
    return true;
}

void Font::InitTexcoords()
{
    if (!mAtlasBitmap.HasContent())
    {
        cxx_assert(false);
        return;
    }

    const Point2D atlasDimensions = mAtlasBitmap.GetDimensions();
    // compute texture coordinates
    float tcx = 1.0f / atlasDimensions.x;
    float tcy = 1.0f / atlasDimensions.y;
    for (CharEntry& roller: mAnsiChars)
    {
        roller.mTexcoords.mU0 = (roller.mRect.x * tcx);
        roller.mTexcoords.mV0 = (roller.mRect.y * tcy);
        roller.mTexcoords.mU1 = (roller.mRect.x + roller.mRect.w) * tcx;
        roller.mTexcoords.mV1 = (roller.mRect.y + roller.mRect.h) * tcy;
    }
    for (auto& roller: mWideChars)
    {
        CharEntry& charEntry = roller.second;
        charEntry.mTexcoords.mU0 = (charEntry.mRect.x * tcx);
        charEntry.mTexcoords.mV0 = (charEntry.mRect.y * tcy);
        charEntry.mTexcoords.mU1 = (charEntry.mRect.x + charEntry.mRect.w) * tcx;
        charEntry.mTexcoords.mV1 = (charEntry.mRect.y + charEntry.mRect.h) * tcy;
    }
}

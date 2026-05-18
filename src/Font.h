#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GpuTexture2D.h"
#include "UiHelpers.h"

//////////////////////////////////////////////////////////////////////////

class Font : public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    static const int AnsiCharsCount = 256;
    static const int MaxAtlasSize = 4096;

    //////////////////////////////////////////////////////////////////////////
    struct CharEntryUV
    {
    public:
        float mU0, mV0;
        float mU1, mV1;
    };
    //////////////////////////////////////////////////////////////////////////
    struct CharEntry
    {
    public:
        Point2D     mOffset; // draw offset
        Rect2D      mRect;  // atlas area
        int         mOuterWidth = 0;
        CharEntryUV mTexcoords;
    };
    //////////////////////////////////////////////////////////////////////////

public:
    Font(const std::string& fontName);

    const std::string& GetName() const { return mFontName; }

    // loads font data from file; does not create render data automatically
    // will initialize default font if loading fails
    void Load();

    // init fallback font
    void InitDefault();

    // uploads system bits to backend; the font must be inited beforehand
    // will init fallback render data on failure
    void InitRenderData();

    // unload system bits and destroy render data 
    void Purge();

    // returns true if either font is loaded from file or it is the default one
    inline bool IsInited() const { return mIsLoadedFromFile || mIsDefaultFont; }
    inline bool IsLoadedFromFile() const { return mIsLoadedFromFile; }
    inline bool IsDefaultFont() const { return mIsDefaultFont; }
    inline bool IsRenderDataInited() const { return mIsRenderDataInited; }

    inline int GetLineHeight() const { return mLineHeight; }

    // create string mesh
    void BuildTextMesh(const std::wstring_view& wideString, const Point2D& pos, Color32 color, std::vector<Quad2D>& outQuads) const;
    void BuildTextMesh(const std::wstring_view& wideString, const Rect2D& rect, 
        eTextHorzAlignment horzAlign, 
        eTextVertAlignment vertAlign, Color32 color, std::vector<Quad2D>& outQuads) const;

    // compute text rect dimensions
    Point2D ComputeTextDims(const std::wstring_view& wideString) const;

    inline GpuTexture2D* GetGpuTexturePtr() const
    {
        return mGpuTextureResource.get();
    }

    inline ePixelFormat GetTexturePixelFormat() const { return mTexturePixelFormat; }

    inline const CharEntry& GetCharEntry(wchar_t ch) const
    {
        if (ch < AnsiCharsCount) return mAnsiChars[ch];

        auto wide_it = mWideChars.find(ch);
        if (wide_it != mWideChars.end())
            return wide_it->second;

        static const CharEntry fallbackEntry {};
        return fallbackEntry;
    }

    int CountPrintableCharacters(const std::wstring_view& wideString) const;

private:
    bool LoadFont_FromFile();
    bool LoadFont_BF4(std::istream& bitstream);
    bool LoadFont_BMF(std::istream& bitstream);

    void InitTexcoords();

private:
    std::string mFontName;

    int mLineHeight = 0;
    
    BitmapImage mAtlasBitmap;
    CharEntry mAnsiChars[AnsiCharsCount];
    std::unordered_map<wchar_t, CharEntry> mWideChars;

    ePixelFormat mTexturePixelFormat = ePixelFormat_Null;

    std::unique_ptr<GpuTexture2D> mGpuTextureResource;

    bool mIsLoadedFromFile = false;
    bool mIsDefaultFont = false; 
    bool mIsRenderDataInited = false;
};
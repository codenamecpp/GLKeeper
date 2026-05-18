#pragma once

//////////////////////////////////////////////////////////////////////////

// combine rgba bytes into single unsigned int value
#define MAKE_RGBA(r,g,b,a) (((unsigned int)(r)) | (((unsigned int)(g)) << 8) | (((unsigned int)(b)) << 16) | (((unsigned int)(a)) << 24))

// predefined rgba colors
enum ColorConstants: unsigned int
{
    COLOR_RED           = MAKE_RGBA(0xFF,0x00,0x00,0xFF),
    COLOR_GREEN         = MAKE_RGBA(0x00,0xFF,0x00,0xFF),
    COLOR_DARK_GREEN    = MAKE_RGBA(0x00,0x80,0x00,0xFF),
    COLOR_ORANGE        = MAKE_RGBA(0xFF,0xA5,0x00,0xFF),
    COLOR_BLUE          = MAKE_RGBA(0x00,0x00,0xFF,0xFF),
    COLOR_DARK_BLUE     = MAKE_RGBA(0x00,0x00,0xA0,0xFF),
    COLOR_WHITE         = MAKE_RGBA(0xFF,0xFF,0xFF,0xFF),
    COLOR_LIGHT_GRAY    = MAKE_RGBA(0xD3,0xD3,0xD3,0xFF),
    COLOR_DARK_GRAY     = MAKE_RGBA(0xA9,0xA9,0xA9,0xFF),
    COLOR_BLACK         = MAKE_RGBA(0x00,0x00,0x00,0xFF),
    COLOR_CYAN          = MAKE_RGBA(0x00,0xFF,0xFF,0xFF),
    COLOR_YELLOW        = MAKE_RGBA(0xFF,0xFF,0x00,0xFF),
    COLOR_PINK          = MAKE_RGBA(0xFF,0x00,0xDC,0xFF),
};

//////////////////////////////////////////////////////////////////////////

// defines rgba color
struct Color32
{
public:
    Color32() = default;
    Color32(unsigned int rgba): mRGBA(rgba) {}
    Color32(unsigned char cr, unsigned char cg, unsigned char cb, unsigned char ca)
        : mR(cr)
        , mG(cg)
        , mB(cb)
        , mA(ca)
    {}

    // @param rgba: Source rgba color
    inline Color32& operator = (const Color32& rgba)
    {
        mRGBA = rgba.mRGBA;
        return *this;
    }

    // Set color components
    // @param theR, theG, theB, theA: Color components
    inline void SetComponents(unsigned char theR, unsigned char theG, unsigned char theB, unsigned char theA)
    {
        mA = theA;
        mR = theR;
        mG = theG;
        mB = theB;
    }

    // Set color components
    // @param theR, theG, theB, theA: Normalized color components in range [0, 1]
    inline void SetComponentsF(float theR, float theG, float theB, float theA)
    {
        mA = (unsigned char) (theA * 255.0f);
        mR = (unsigned char) (theR * 255.0f);
        mG = (unsigned char) (theG * 255.0f);
        mB = (unsigned char) (theB * 255.0f);
    }

    // conversion to vec3
    inline glm::vec3 ToFloats3() const
    {
        static const float inv = 1.0f / 255.0f;
        return { mR * inv, mG * inv, mB * inv };
    }

    // conversion to vec4
    inline glm::vec4 ToFloats4() const
    {
        static const float inv = 1.0f / 255.0f;
        return { mR * inv, mG * inv, mB * inv, mA * inv }; 
    }

    // implicit conversion to int
    inline operator unsigned int () const { return mRGBA; }

    inline unsigned char operator [] (int index) const
    {
        cxx_assert((index > -1) && (index < MaxChannels));
        return mChannels[index];
    }

    inline unsigned char& operator [] (int index)
    {
        cxx_assert((index > -1) && (index < MaxChannels));
        return mChannels[index];
    }

    inline bool operator == (const Color32& rhs) { return mRGBA == rhs.mRGBA; }
    inline bool operator != (const Color32& rhs) { return !(*this == rhs); }

public:
    static constexpr int MaxChannels = 4;
    union
    {
        struct
        {
            unsigned char mChannels[MaxChannels];
        };
        struct
        {
            unsigned char mR;
            unsigned char mG;
            unsigned char mB;
            unsigned char mA;
        };
        unsigned int mRGBA;
    };
};

enum
{
    Sizeof_Color32 = sizeof(Color32)
};

//////////////////////////////////////////////////////////////////////////

// log messages levels
enum eLogLevel
{
    eLogLevel_Debug,
    eLogLevel_Info,
    eLogLevel_Warning,
    eLogLevel_Error,
    eLogLevel_COUNT
};

enum_serialize_decl(eLogLevel);

//////////////////////////////////////////////////////////////////////////

using Point2D = glm::ivec2;

//////////////////////////////////////////////////////////////////////////

// defines rectangle in 2d space
struct Rect2D
{
public:
    Rect2D() = default;
    Rect2D(int posx, int posy, int sizex, int sizey)
        : x(posx), y(posy)
        , w(sizex), h(sizey)
    {
    }

    inline void SetToZero()
    {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
    }
    // test whether point is within rect
    inline bool PointWithin(const Point2D& point) const
    {
        return point.x >= x && point.y >= y &&
            point.x < (x + w - 1) &&
            point.y < (y + h - 1);
    }
    // get union area of two rectangles
    inline Rect2D GetUnion(const Rect2D& rc) const
    {
        Rect2D rcOutput;

        int maxx = glm::max(x + w, rc.x + rc.w);
        int maxy = glm::max(y + h, rc.y + rc.h);

        rcOutput.x = glm::min(x, rc.x);
        rcOutput.y = glm::min(y, rc.y);
        rcOutput.w = glm::max(maxx - rcOutput.x, 0);
        rcOutput.h = glm::max(maxy - rcOutput.y, 0);

        return rcOutput;
    }
    // get intersection area of two rectangles
    inline Rect2D GetIntersection(const Rect2D& rc) const
    {        
        Rect2D rcOutput;

        int minx = glm::min(x + w, rc.x + rc.w);
        int miny = glm::min(y + h, rc.y + rc.h);

        rcOutput.x = glm::max(x, rc.x);
        rcOutput.y = glm::max(y, rc.y);
        rcOutput.w = glm::max(minx - rcOutput.x, 0);
        rcOutput.h = glm::max(miny - rcOutput.y, 0);

        return rcOutput;
    }

    inline bool operator == (const Rect2D& rhs) const { return (x == rhs.x) && (y == rhs.y) && (w == rhs.w) && (h == rhs.h); }
    inline bool operator != (const Rect2D& rhs) const { return !(*this == rhs); }
public:
    int x, y;
    int w, h;
};

//////////////////////////////////////////////////////////////////////////

// dynamic transient buffer
// warning: all allocated data is invalidated at the start of the next frame
// memory is reclaimed every update frame, see FrameMemoryManager

template<typename T>
using Temp_Vector = std::pmr::vector<T>;

template<typename T>
using Temp_Set = std::pmr::set<T>;

template<typename T>
using Temp_List = std::pmr::list<T>;

template<typename T, typename TContainer>
inline auto MakeTempVector(const TContainer& sourceContainer)
{
    Temp_Vector<T> tempVector;
    tempVector.assign(std::begin(sourceContainer), std::end(sourceContainer));
    return std::move(tempVector);
}

template<typename T>
inline auto MakeTempVector(const cxx::span<T>& sourceContainer)
{
    Temp_Vector<T> tempVector;
    tempVector.assign(std::begin(sourceContainer), std::end(sourceContainer));
    return std::move(tempVector);
}

//////////////////////////////////////////////////////////////////////////
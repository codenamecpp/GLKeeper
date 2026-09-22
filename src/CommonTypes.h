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

inline bool operator < (const Point2D& lhs, const Point2D& rhs)
{
    return (lhs.y != rhs.y) ? (lhs.y < rhs.y) : (lhs.x < rhs.x);
}

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

    static Rect2D FromPoints(const Point2D& pta, const Point2D& ptb)
    {
        Rect2D rc;
        rc.SetFromPoints(pta, ptb);
        return rc;
    }

    inline void SetFromPoints(const Point2D& pta, const Point2D& ptb)
    {
        const Point2D pmin = glm::min(pta, ptb);
        const Point2D pmax = glm::max(pta, ptb);
        x = pmin.x;
        y = pmin.y;
        w = pmax.x - pmin.x + 1;
        h = pmax.y - pmin.y + 1;
    }

    inline void SetPosition(const Point2D& rectPosition)
    {
        x = rectPosition.x;
        y = rectPosition.y;
    }

    inline void SetSize(const Point2D& rectSize)
    {
        w = std::max(0, rectSize.x);
        h = std::max(0, rectSize.y);
    }

    inline bool Empty() const { return !HasSize(); }
    inline bool HasSize() const
    {
        return (w > 0) && (h > 0);
    }

    inline bool IsPoint() const { return (w == 1) && (h == 1); }

    inline void SetToZero()
    {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
    }

    inline void Inflate(const Point2D& inflateSize)
    {
        x -= inflateSize.x;
        y -= inflateSize.y;
        w += inflateSize.x * 2;
        h += inflateSize.y * 2;
    }

    inline Point2D GetCenter() const
    {
        return Point2D 
        { 
            x + (w >> 1),
            y + (h >> 1) 
        };
    }

    inline Point2D GetPosition() const { return {x, y}; }
    inline Point2D GetSize() const { return {w, h}; }

    inline void Translate(const Point2D& offset)
    {
        x += offset.x;
        y += offset.y;
    }

    inline Rect2D GetTranslated(const Point2D& offset) const
    {
        return Rect2D{ x + offset.x, y + offset.y, w, h };
    }

    inline Rect2D GetInflated(const Point2D& inflateSize) const
    {
        Rect2D rc {x, y, w, h}; 
        rc.Inflate(inflateSize); 
        return rc;
    }

    // whether point is within rect
    inline bool ContainsPoint(const Point2D& point) const
    {
        if (point.x < x) return false;
        if (point.y < y) return false;
        return ((point.x - x) < w) && ((point.y - y) < h);
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

    // get intersection area of two rectangular map areas
    inline Rect2D GetIntersection(const Rect2D& rhs) const
    {
        // make sure both rectangles has area
        if (HasSize() && rhs.HasSize())
        {
            // calc boundaries of the intersection interval [x1, x2), [y1, y2)

            const int x1 = std::max(x, rhs.x);
            const int y1 = std::max(y, rhs.y);
            const int x2 = std::min(x + w, rhs.x + rhs.w);
            const int y2 = std::min(y + h, rhs.y + rhs.h);

            // check intersection
            if ((x1 < x2) && (y1 < y2))
            {
                return Rect2D {x1, y1, x2 - x1, y2 - y1};
            }
        }
        return Rect2D {0, 0, 0, 0};
    }

    inline bool operator == (const Rect2D& rhs) const { return (x == rhs.x) && (y == rhs.y) && (w == rhs.w) && (h == rhs.h); }
    inline bool operator != (const Rect2D& rhs) const { return !(*this == rhs); }
public:
    int x, y;
    int w, h;
};

//////////////////////////////////////////////////////////////////////////

inline Rect2D FitAspectContain(const Rect2D& src, const Rect2D& dst)
{
    if (dst.Empty() || src.Empty())
    {
        return {};
    }

    float scaleX = (dst.w * 1.0f) / (src.w * 1.0f);
    float scaleY = (dst.h * 1.0f) / (src.h * 1.0f);

    float minScale = std::min(scaleX, scaleY);

    Rect2D rcResult;
    rcResult.w = static_cast<int>(src.w * minScale);
    rcResult.h = static_cast<int>(src.h * minScale);
    rcResult.x = dst.x + ((dst.w - rcResult.w) >> 1);
    rcResult.y = dst.y + ((dst.h - rcResult.h) >> 1);
    return rcResult;
}

//////////////////////////////////////////////////////////////////////////

template<auto Enum_COUNT>
struct EnumSet
{
    static_assert(std::is_enum_v<decltype(Enum_COUNT)>, "Enum_COUNT must be an enum!");

public:
    using EnumType = std::decay_t<decltype(Enum_COUNT)>;

public:
    constexpr EnumSet() = default;
    constexpr EnumSet(EnumType enumValue) 
    {
        Include(enumValue);
    }
    constexpr EnumSet(std::initializer_list<EnumType> enumValues) 
    {
        for (EnumType val : enumValues) 
        {
            Include(val);
        }
    }
    constexpr EnumSet& operator = (const EnumSet& other) { mBitset = other.mBitset; return *this; }
    constexpr EnumSet& operator = (EnumType enumValue)
    {
        return Assing(enumValue);
    }
    constexpr EnumSet& Assing(EnumType enumValue) 
    {  
        mBitset.reset().set(enumValue);
        return *this;
    }
    constexpr EnumSet& Set(EnumType enumValue, bool flag)
    {
        mBitset.set(enumValue, flag);
        return *this;
    }
    constexpr bool Empty() const { return mBitset.none(); }
    constexpr bool Contains(EnumType enumValue) const 
    { 
        return mBitset.test(static_cast<std::size_t>(enumValue)); 
    }
    constexpr bool HasAny(const EnumSet& other) const { return (mBitset & other.mBitset).any(); }
    constexpr bool HasAll(const EnumSet& other) const { return (mBitset & other.mBitset) == other.mBitset; }

    constexpr EnumSet& Clear() { mBitset.reset(); return *this; }
    constexpr EnumSet& Include(EnumType enumValue) 
    { 
        mBitset.set(static_cast<std::size_t>(enumValue), true); 
        return *this; 
    }
    constexpr EnumSet& Exclude(EnumType enumValue) 
    { 
        mBitset.set(static_cast<std::size_t>(enumValue), false); 
        return *this; 
    }

    constexpr EnumSet operator | (const EnumSet& other) const { return EnumSet{mBitset | other.mBitset}; }
    constexpr EnumSet operator & (const EnumSet& other) const { return EnumSet{mBitset & other.mBitset}; }

    constexpr EnumSet& operator |= (EnumType enumValue) { return Include(enumValue); }
    constexpr EnumSet& operator |= (const EnumSet& other) { mBitset |= other.mBitset; return *this; }
    constexpr EnumSet& operator &= (const EnumSet& other) { mBitset &= other.mBitset; return *this; }

    constexpr EnumSet operator ~() const { return EnumSet{~mBitset}; }

    constexpr bool operator == (const EnumSet& other) const { return mBitset == other.mBitset; }
    constexpr bool operator != (const EnumSet& other) const { return !(*this == other); }
private:
    explicit constexpr EnumSet(const std::bitset<static_cast<std::size_t>(Enum_COUNT)>& bitset)
        : mBitset(bitset) 
    {
    }
private:
    std::bitset<static_cast<std::size_t>(Enum_COUNT)> mBitset;
};

//////////////////////////////////////////////////////////////////////////

using StringHash = size_t;

inline StringHash HashForString(std::string_view srcString)
{
    return std::hash<std::string_view> {}(srcString);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

namespace cxx
{

    // temporary / frame / transient containers

    // warning: all allocated data is invalidated at the start of the next frame
    // memory is reclaimed every update frame, see FrameMemoryManager

    template<typename T> using temp_vector = std::pmr::vector<T>;
    template<typename T> using temp_set = std::pmr::set<T>;
    template<typename T> using temp_list = std::pmr::list<T>;
    template<typename TKey, typename TValue, typename TLess = std::less<TKey>> 
    using temp_map = std::pmr::map<TKey, TValue, TLess>;

    // helpers

    template<template<typename...> class TContainer, typename T, typename... Args>
    inline auto temp_vector_from(const TContainer<T, Args...>& sourceContainer)
    {
        temp_vector<T> tempVector;
        tempVector.assign(std::begin(sourceContainer), std::end(sourceContainer));
        return std::move(tempVector);
    }

    template<typename T>
    inline auto temp_vector_from(const cxx::span<T>& sourceContainer)
    {
        temp_vector<T> tempVector;
        tempVector.assign(std::begin(sourceContainer), std::end(sourceContainer));
        return std::move(tempVector);
    }

} // namespace cxx

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
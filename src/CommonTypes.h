#pragma once

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
    {
    }
    // @param rgba: Source 32bits color
    inline Color32& operator = (const Color32& rgba)
    {
        mRGBA = rgba.mRGBA;
        return *this;
    }
    // set color components
    // @param cr, cg, cb, ca: Color components
    inline void Setup(unsigned char cr, unsigned char cg, unsigned char cb, unsigned char ca)
    {
        mA = cr;
        mR = cg;
        mG = cg;
        mB = cb;
    }

    // combine rgba channels into single unsigned int value
    static unsigned int MakeRGBA(unsigned char cr, unsigned char cg, unsigned char cb, unsigned char ca)
    {
        return ((unsigned int)(cr)) | (((unsigned int)(cg)) << 8U) | (((unsigned int)(cb)) << 16U) | (((unsigned int)(ca)) << 24U);
    }

    // implicit conversion to int
    inline operator unsigned int () const { return mRGBA; }

public:
    union
    {
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

const unsigned int Sizeof_Color32 = sizeof(Color32);

inline bool operator == (const Color32& lhs, const Color32& rhs) { return lhs.mRGBA == rhs.mRGBA; }
inline bool operator != (const Color32& lhs, const Color32& rhs) { return lhs.mRGBA != rhs.mRGBA; }

// standard colors
extern const Color32 Color32_Red;
extern const Color32 Color32_Green;
extern const Color32 Color32_DarkGreen;
extern const Color32 Color32_Orange;
extern const Color32 Color32_Blue;
extern const Color32 Color32_Brown;
extern const Color32 Color32_SkyBlue;
extern const Color32 Color32_DarkBlue;
extern const Color32 Color32_White;
extern const Color32 Color32_DarkGray;
extern const Color32 Color32_GrimGray;
extern const Color32 Color32_Gray;
extern const Color32 Color32_Black;
extern const Color32 Color32_Cyan;
extern const Color32 Color32_Yellow;

// defines array of bytes
using ByteArray = std::vector<unsigned char>;

// defines point in 2d space
using Point2D = glm::ivec2;

// defines size in 2d space
using Size2D = glm::ivec2;

// defines rectangle in 2d space
struct Rect2D
{
public:
    Rect2D() = default;
    Rect2D(int posx, int posy, int sizex, int sizey)
        : mX(posx)
        , mY(posy)
        , mSizeX(sizex)
        , mSizeY(sizey)
    {
    }
    inline void Set(int posx, int posy, int sizex, int sizey)
    {
        mX = posx;
        mY = posy;
        mSizeX = sizex;
        mSizeY = sizey;
    }
    inline void SetNull()
    {
        mX = 0;
        mY = 0;
        mSizeX = 0;
        mSizeY = 0;
    }
public:
    int mX, mY;
    int mSizeX, mSizeY;
};

inline bool operator == (const Rect2D& lhs, const Rect2D& rhs) 
{ 
    return (lhs.mX == rhs.mX) && (lhs.mY == rhs.mY) && 
        (lhs.mSizeX == rhs.mSizeX) && 
        (lhs.mSizeY == rhs.mSizeY);
}

inline bool operator != (const Rect2D& lhs, const Rect2D& rhs)
{
    return (lhs.mX != rhs.mX) || (lhs.mY != rhs.mY) || 
        (lhs.mSizeX != rhs.mSizeX) ||
        (lhs.mSizeY != rhs.mSizeY);
}

// console log message category
enum eLogMessage
{
    eLogMessage_Debug,
    eLogMessage_Info,
    eLogMessage_Warning,
    eLogMessage_Error,
    eLogMessage_COUNT
};

decl_enum_strings(eLogMessage);

// 
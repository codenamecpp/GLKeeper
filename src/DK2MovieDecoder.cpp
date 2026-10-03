#include "stdafx.h"
#include "DK2MovieDecoder.h"

//////////////////////////////////////////////////////////////////////////

inline uint16_t rl16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline uint32_t rl32(const uint8_t* p) 
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
inline int8_t s8(uint8_t v) { return (int8_t)v; }
inline uint8_t clip8(int v) 
{
    if (v <   0) return   0;
    if (v > 255) return 255;
    return (uint8_t) v;
}
inline int align16(int v) { return (v + 15) & ~15; }

// MPEG zigzag (raster index for scan position)
static const uint8_t kZigzag[64] = 
{
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// AAN inverse scales (used by EA TGQ/TQI quant matrices)
static const uint16_t kInvAan[64] = 
{
    4096,  2953,  3135,  3483,  4096,  5213,  7568, 14846,
    2953,  2129,  2260,  2511,  2953,  3759,  5457, 10703,
    3135,  2260,  2399,  2666,  3135,  3990,  5793, 11363,
    3483,  2511,  2666,  2962,  3483,  4433,  6436, 12625,
    4096,  2953,  3135,  3483,  4096,  5213,  7568, 14846,
    5213,  3759,  3990,  4433,  5213,  6635,  9633, 18895,
    7568,  5457,  5793,  6436,  7568,  9633, 13985, 27432,
   14846, 10703, 11363, 12625, 14846, 18895, 27432, 53809
};

// MPEG-1 default intra quant matrix
static const uint8_t kMpeg1Intra[64] = 
{
     8, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83
};

// MPEG-1 DCT coefficient VLC (ISO 11172-2 table B.5), plus escape / EOB.
// code, bits, run, level. 111 entries + escape + eob.
struct MpegAc 
{
    uint16_t code;
    uint8_t bits;
    uint8_t run;
    int8_t level; // 0 = EOB if run==0 && code is EOB; escape marked separately
};

static const MpegAc kMpeg1Ac[] = 
{
    {0x3,2,0,1},{0x4,4,0,2},{0x5,5,0,3},{0x6,7,0,4},
    {0x26,8,0,5},{0x21,8,0,6},{0xa,10,0,7},{0x1d,12,0,8},
    {0x18,12,0,9},{0x13,12,0,10},{0x10,12,0,11},{0x1a,13,0,12},
    {0x19,13,0,13},{0x18,13,0,14},{0x17,13,0,15},{0x1f,14,0,16},
    {0x1e,14,0,17},{0x1d,14,0,18},{0x1c,14,0,19},{0x1b,14,0,20},
    {0x1a,14,0,21},{0x19,14,0,22},{0x18,14,0,23},{0x17,14,0,24},
    {0x16,14,0,25},{0x15,14,0,26},{0x14,14,0,27},{0x13,14,0,28},
    {0x12,14,0,29},{0x11,14,0,30},{0x10,14,0,31},{0x18,15,0,32},
    {0x17,15,0,33},{0x16,15,0,34},{0x15,15,0,35},{0x14,15,0,36},
    {0x13,15,0,37},{0x12,15,0,38},{0x11,15,0,39},{0x10,15,0,40},
    {0x3,3,1,1},{0x6,6,1,2},{0x25,8,1,3},{0xc,10,1,4},
    {0x1b,12,1,5},{0x16,13,1,6},{0x15,13,1,7},{0x1f,15,1,8},
    {0x1e,15,1,9},{0x1d,15,1,10},{0x1c,15,1,11},{0x1b,15,1,12},
    {0x1a,15,1,13},{0x19,15,1,14},{0x13,16,1,15},{0x12,16,1,16},
    {0x11,16,1,17},{0x10,16,1,18},{0x5,4,2,1},{0x4,7,2,2},
    {0xb,10,2,3},{0x14,12,2,4},{0x14,13,2,5},{0x7,5,3,1},
    {0x24,8,3,2},{0x1c,12,3,3},{0x13,13,3,4},{0x6,5,4,1},
    {0xf,10,4,2},{0x12,12,4,3},{0x7,6,5,1},{0x9,10,5,2},
    {0x12,13,5,3},{0x5,6,6,1},{0x1e,12,6,2},{0x14,16,6,3},
    {0x4,6,7,1},{0x15,12,7,2},{0x7,7,8,1},{0x11,12,8,2},
    {0x5,7,9,1},{0x11,13,9,2},{0x27,8,10,1},{0x10,13,10,2},
    {0x23,8,11,1},{0x1a,16,11,2},{0x22,8,12,1},{0x19,16,12,2},
    {0x20,8,13,1},{0x18,16,13,2},{0xe,10,14,1},{0x17,16,14,2},
    {0xd,10,15,1},{0x16,16,15,2},{0x8,10,16,1},{0x15,16,16,2},
    {0x1f,12,17,1},{0x1a,12,18,1},{0x19,12,19,1},{0x17,12,20,1},
    {0x16,12,21,1},{0x1f,13,22,1},{0x1e,13,23,1},{0x1d,13,24,1},
    {0x1c,13,25,1},{0x1b,13,26,1},{0x1f,16,27,1},{0x1e,16,28,1},
    {0x1d,16,29,1},{0x1c,16,30,1},{0x1b,16,31,1},
};

// 16-bit MSB peek LUT: bits(5) | run(6) | kind(2) | level(8 signed stored +128)
// kind: 0=invalid 1=coeff 2=eob 3=escape
struct AcLut { uint8_t bits, run, kind; int8_t level; };

inline const AcLut* ac_lut() 
{
    static AcLut lut[65536];
    static bool ready = false;
    if (ready) 
    {
        return lut;
    }
    std::memset(lut, 0, sizeof(lut));
    auto fill = [&](uint16_t code, int bits, uint8_t run, uint8_t kind, int8_t level) 
        {
            const int shift = 16 - bits;
            const int start = (int)code << shift;
            const int count = 1 << shift;
            for (int i = 0; i < count; i++) 
            {
                lut[start + i].bits = (uint8_t)bits;
                lut[start + i].run = run;
                lut[start + i].kind = kind;
                lut[start + i].level = level;
            }
        };
    for (const auto& e : kMpeg1Ac)
    {
        fill(e.code, e.bits, e.run, 1, e.level);
    }
    fill(0x1, 6, 0, 3, 0); // escape 000001
    fill(0x2, 2, 0, 2, 0); // EOB 10
    ready = true;
    return lut;
}

// MPEG-1 DC size VLCs (lum / chroma)
inline int decode_dc_size_lum(class BitMsb& b);
inline int decode_dc_size_chr(class BitMsb& b);

// ---- bitstream: TQI = LE 32-bit words, bits taken MSB-first of each word
class BitMsb 
{
public:
    const uint8_t* mDataPtr;
    uint32_t mLeft;
    uint32_t mCache {};
    int mNumBits {};

public:
    BitMsb(const uint8_t* data, uint32_t size) 
        : mDataPtr(data)
        , mLeft(size) 
    {
    }

    inline void refill() 
    {
        if (mNumBits > 0) 
            return;

        mNumBits = 32;

        if (mLeft >= 4) 
        {
            mCache = rl32(mDataPtr);
            mDataPtr += 4;
            mLeft -= 4;
            return;
        }

        if (mLeft == 0) 
        {
            mCache = 0;
            return;
        }

        // leftover < 4: treat remaining bytes as a short LE word, high bytes 0
        uint32_t w = 0;
        for (uint32_t i = 0; i < mLeft; i++) 
        {
            w |= (uint32_t)mDataPtr[i] << (8 * i);
        }
        mDataPtr += mLeft;
        mLeft = 0;
        mCache = w;
    }

    inline uint32_t get(int n) 
    {
        uint32_t v = 0;
        while (n > 0) 
        {
            refill();
            const int take = n < mNumBits ? n : mNumBits;
            v = (v << take) | (mCache >> (32 - take));
            mCache <<= take;
            mNumBits -= take;
            n -= take;
        }
        return v;
    }

    inline uint32_t peek(int n) 
    {
        // refill into a local copy
        BitMsb t = *this;
        return t.get(n);
    }

    inline int get_s(int n) 
    {
        const int v = (int)get(n);
        const int sh = 32 - n;
        return (v << sh) >> sh;
    }

    inline int mpeg_signed(int size) 
    {
        if (size <= 0) 
            return 0;

        const int val = (int)get(size);
        const int sign = (val >> (size - 1)) ^ 1;
        return val + sign - (sign << size);
    }
};

inline int decode_dc_size_lum(BitMsb& b) 
{
    // 00=1, 01=2, 100=0, 101=3, 110=4, 1110=5, ... 111111111=11
    if (b.peek(2) < 2) 
    {
        const int v = (int)b.get(2);
        return v + 1; // 00->1, 01->2
    }

    if (b.peek(3) == 4) { b.get(3); return 0; }      // 100
    if (b.peek(3) == 5) { b.get(3); return 3; }      // 101
    if (b.peek(3) == 6) { b.get(3); return 4; }      // 110

    // 111...
    b.get(3);
    int size = 5;
    while (size < 11) 
    {
        if (b.get(1) == 0) 
            return size;

        size++;
    }
    return 11; // 111111111
}

inline int decode_dc_size_chr(BitMsb& b) 
{
    // 00=0, 01=1, 10=2, 110=3, 1110=4, ... 1111111111=11
    const int t2 = (int) b.peek(2);
    if (t2 <= 2) 
    {
        b.get(2);
        return t2; // 00,01,10
    }

    b.get(2); // 11

    int size = 3;
    while (size < 11) 
    {
        if (b.get(1) == 0) 
            return size;

        size++;
    }
    return 11;
}

// ---- bitstream: TGQ = LE, bits taken LSB-first
class BitLsb 
{
public:
    const uint8_t* mDataPtr;
    uint32_t mLeft;
    uint32_t mCache {};
    int mNumBits {};

public:
    BitLsb(const uint8_t* data, uint32_t size) 
        : mDataPtr(data)
        , mLeft(size) 
    {
    }

    inline void refill() 
    {
        while ((mNumBits <= 24) && mLeft) 
        {
            mCache |= (uint32_t)(*mDataPtr++) << mNumBits;
            mLeft--;
            mNumBits += 8;
        }
    }

    inline uint32_t peek(int n) 
    {
        refill();
        return mCache & ((1u << n) - 1u);
    }

    inline uint32_t get(int n) 
    {
        refill();
        const uint32_t v = mCache & ((1u << n) - 1u);
        mCache >>= n;
        mNumBits -= n;
        return v;
    }

    inline int get_s(int n) 
    {
        const int v = (int)get(n);
        const int sh = 32 - n;
        return (v << sh) >> sh;
    }
};

// ---- EA IDCT (same integer kernel as eaidct.c)
constexpr int kAsqrt = 181;
constexpr int kA4 = 669;
constexpr int kA2 = 277;
constexpr int kA5 = 196;

inline void idct_col(int16_t* dest, const int16_t* src)
{
    if ((src[8] | src[16] | src[24] | src[32] | src[40] | src[48] | src[56]) == 0) 
    {
        dest[0] = dest[8] = dest[16] = dest[24] = dest[32] = dest[40] = dest[48] = dest[56] = src[0];
        return;
    }
    const int a1 = src[8] + src[56];
    const int a7 = src[8] - src[56];
    const int a5 = src[40] + src[24];
    const int a3 = src[40] - src[24];
    const int a2 = src[16] + src[48];
    const int a6 = (kAsqrt * (src[16] - src[48])) >> 8;
    const int a0 = src[0] + src[32];
    const int a4 = src[0] - src[32];
    const int b0 = (((kA4 - kA5) * a7 - kA5 * a3) >> 9) + a1 + a5;
    const int b1 = (((kA4 - kA5) * a7 - kA5 * a3) >> 9) + ((kAsqrt * (a1 - a5)) >> 8);
    const int b2 = (((kA2 + kA5) * a3 + kA5 * a7) >> 9) + ((kAsqrt * (a1 - a5)) >> 8);
    const int b3 = ((kA2 + kA5) * a3 + kA5 * a7) >> 9;
    dest[0]  = (int16_t)(a0 + a2 + a6 + b0);
    dest[8]  = (int16_t)(a4 + a6 + b1);
    dest[16] = (int16_t)(a4 - a6 + b2);
    dest[24] = (int16_t)(a0 - a2 - a6 + b3);
    dest[32] = (int16_t)(a0 - a2 - a6 - b3);
    dest[40] = (int16_t)(a4 - a6 - b2);
    dest[48] = (int16_t)(a4 + a6 - b1);
    dest[56] = (int16_t)(a0 + a2 + a6 - b0);
}

inline void ea_idct_put(uint8_t* dest, int stride, int16_t block[64]) 
{
    int16_t temp[64];
    block[0] = (int16_t)(block[0] + 4);
    for (int i = 0; i < 8; i++)
    {
        idct_col(&temp[i], &block[i]);
    }
    for (int i = 0; i < 8; i++) 
    {
        const int16_t* src = &temp[8 * i];
        const int a1 = src[1] + src[7];
        const int a7 = src[1] - src[7];
        const int a5 = src[5] + src[3];
        const int a3 = src[5] - src[3];
        const int a2 = src[2] + src[6];
        const int a6 = (kAsqrt * (src[2] - src[6])) >> 8;
        const int a0 = src[0] + src[4];
        const int a4 = src[0] - src[4];
        const int b0 = (((kA4 - kA5) * a7 - kA5 * a3) >> 9) + a1 + a5;
        const int b1 = (((kA4 - kA5) * a7 - kA5 * a3) >> 9) + ((kAsqrt * (a1 - a5)) >> 8);
        const int b2 = (((kA2 + kA5) * a3 + kA5 * a7) >> 9) + ((kAsqrt * (a1 - a5)) >> 8);
        const int b3 = ((kA2 + kA5) * a3 + kA5 * a7) >> 9;
        dest[0] = clip8((a0 + a2 + a6 + b0) >> 4);
        dest[1] = clip8((a4 + a6 + b1) >> 4);
        dest[2] = clip8((a4 - a6 + b2) >> 4);
        dest[3] = clip8((a0 - a2 - a6 + b3) >> 4);
        dest[4] = clip8((a0 - a2 - a6 - b3) >> 4);
        dest[5] = clip8((a4 - a6 - b2) >> 4);
        dest[6] = clip8((a4 + a6 - b1) >> 4);
        dest[7] = clip8((a0 + a2 + a6 - b0) >> 4);
        dest += stride;
    }
}

inline void idct_put_mb(uint8_t* y, int ystride, uint8_t* cb, uint8_t* cr, int cstride, int16_t block[6][64]) 
{
    ea_idct_put(y, ystride, block[0]);
    ea_idct_put(y + 8, ystride, block[1]);
    ea_idct_put(y + 8 * ystride, ystride, block[2]);
    ea_idct_put(y + 8 * ystride + 8, ystride, block[3]);
    ea_idct_put(cb, cstride, block[4]);
    ea_idct_put(cr, cstride, block[5]);
}

// TQI intra block. DC scaled by matrix[0]
inline bool tqi_block(BitMsb& b, int16_t block[64], int n, int last_dc[3], const uint16_t matrix[64]) 
{
    std::memset(block, 0, 64 * sizeof(int16_t));

    const int comp = (n < 4) ? 0 : ((n & 1) + 1);
    const int size = (comp == 0) ? decode_dc_size_lum(b) : decode_dc_size_chr(b);

    if (size < 0 || size > 11) 
        return false;

    const int diff = (size == 0) ? 0 : b.mpeg_signed(size);
    last_dc[comp] += diff;

    block[0] = (int16_t)(last_dc[comp] * (int)matrix[0]);

    const AcLut* lut = ac_lut();
    int i = 0;
    for (;;) 
    {
        const uint32_t peekData = b.peek(16);
        const AcLut e = lut[peekData & 0xFFFF];
        if (e.kind == 0 || e.bits == 0) 
        {
            return false;
        }

        b.get(e.bits);

        if (e.kind == 2) 
            break; // EOB

        int level;
        if (e.kind == 3) 
        {
            const int run = (int)b.get(6) + 1;
            level = b.get_s(8);
            if (level == -128) 
            {
                level = (int)b.get(8) - 256;
            }
            else if (level == 0) 
            {
                level = (int)b.get(8);
            }
            i += run;

            if (i < 0 || i >= 64) 
                return false;

            const int j = kZigzag[i];
            int mag = level < 0 ? -level : level;
            mag = (mag * (int)matrix[j]) >> 4;
            mag = (mag - 1) | 1;
            block[j] = (int16_t)(level < 0 ? -mag : mag);
        } 
        else 
        {
            i += (int)e.run + 1;
            if (i < 0 || i >= 64) 
                return false;

            const int j = kZigzag[i];
            int mag = (int)e.level;
            mag = (mag * (int)matrix[j]) >> 4;
            mag = (mag - 1) | 1;
            const int sign = (int)b.get(1);
            block[j] = (int16_t)(sign ? -mag : mag);
        }
    }
    return true;
}

inline void tqi_qtable(uint16_t matrix[64], int quant) 
{
    const int64_t qscale = (215 - 2 * quant) * 5;
    matrix[0] = (uint16_t)((kInvAan[0] * kMpeg1Intra[0]) >> 11);
    for (int i = 1; i < 64; i++)
    {
        int64_t v = ((int64_t)kInvAan[i] * kMpeg1Intra[i] * qscale + 32) >> 14;
        matrix[i] = (uint16_t)(v > 65535 ? 65535 : v);
    }
}

inline void tgq_qtable(int qtable[64], int quant) 
{
    const int a = (14 * (100 - quant)) / 100 + 1;
    const int b = (11 * (100 - quant)) / 100 + 4;
    for (int j = 0; j < 8; j++)
    {
        for (int i = 0; i < 8; i++)
        {
            qtable[j * 8 + i] = ((a * (j + i) / 14 + b) * (int)kInvAan[j * 8 + i]) >> 10;
        }
    }
}

inline bool tgq_block(BitLsb& b, int16_t block[64], const int qtable[64]) 
{
    std::memset(block, 0, 64 * sizeof(int16_t));
    block[0] = (int16_t)(b.get_s(8) * qtable[0]);

    for (int i = 1; i < 64;) 
    {
        switch (b.peek(3)) 
        {
            case 4:
            {
                if (i >= 63) 
                    return false;

                block[kZigzag[i++]] = 0;
                // fallthrough
            }
            case 0:
            {
                block[kZigzag[i++]] = 0;
                b.get(3);
                break;
            }
            case 5:
            case 1: 
            {
                b.get(2);
                const int value = (int)b.get(6);
                if (value > 64 - i) 
                    return false;

                for (int j = 0; j < value; j++)
                {
                    block[kZigzag[i++]] = 0;
                }
                break;
            }
            case 6:
            {
                b.get(3);
                block[kZigzag[i]] = (int16_t)(-qtable[kZigzag[i]]);
                i++;
                break;
            }
            case 2:
            {
                b.get(3);
                block[kZigzag[i]] = (int16_t)(qtable[kZigzag[i]]);
                i++;
                break;
            }
            case 7:
            case 3:
            {
                b.get(2);
                if (b.peek(6) == 0x3F) 
                {
                    b.get(6);
                    block[kZigzag[i]] = (int16_t)(b.get_s(8) * qtable[kZigzag[i]]);
                } 
                else 
                {
                    block[kZigzag[i]] = (int16_t)(b.get_s(6) * qtable[kZigzag[i]]);
                }
                i++;
                break;
            }
        }
    }
    block[0] = (int16_t)(block[0] + (128 << 4));
    return true;
}

inline void fill8(uint8_t* dst, int stride, uint8_t v) 
{
    for (int j = 0; j < 8; j++)
    {
        std::memset(dst + j * stride, v, 8);
    }
}

inline void copy16(uint8_t* dst, int dstride, const uint8_t* src, int sstride, int h, int w) 
{
    for (int j = 0; j < h; j++)
    {
        std::memcpy(dst + j * dstride, src + j * sstride, (uint32_t)w);
    }
}

//////////////////////////////////////////////////////////////////////////

bool DK2MovieDecoder::OpenFile(const std::string& path) 
{
    std::ifstream fileStream(path, std::ios::binary);
    if (!fileStream) 
    {
        return false;
    }
    fileStream.seekg(0, std::ios::end);
    const auto n = fileStream.tellg();
    if (n <= 0) 
    {
        return false;
    }
    fileStream.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf((uint32_t)n);
    fileStream.read((char*) buf.data(), n);
    if (!fileStream) 
    {
        return false;
    }
    return OpenMemory(buf.data(), buf.size());
}

bool DK2MovieDecoder::OpenMemory(const uint8_t* data, uint32_t size) 
{
    Close();
    if (size < 8) // too small
    {
        mErrorCode = eErrorCode_FileError;
        return false;
    }
    mFileData.assign(data, data + size);

    DemuxResult demuxNextResult;
    if (!DemuxNext(demuxNextResult)) 
    {
        if (mErrorCode == eErrorCode_None)
        {
            mErrorCode = eErrorCode_EOF;
        }
        return false;
    }
    return true;
}

void DK2MovieDecoder::Close()
{
    mFileData.clear();
    mFilePosition = 0;

    mCodec = {};
    mErrorCode = {};

    mFrameDims = {};
    mFrameDimsCoded = {};

    mFrameIndex = 0;

    mY.clear();
    mCb.clear();
    mCr.clear();
    mLastY.clear();
    mLastCb.clear();
    mLastCr.clear();

    mHaveReference = false;
}

void DK2MovieDecoder::Rewind() 
{
    mFilePosition = 0;
    mFrameIndex = 0;
    mHaveReference = false;
}

bool DK2MovieDecoder::DemuxNext(DemuxResult& outResult) 
{
    while (mFilePosition + 8 <= mFileData.size()) 
    {
        outResult.mTag = rl32(mFileData.data() + mFilePosition);

        const uint32_t raw = rl32(mFileData.data() + mFilePosition + 4);

        if (raw < 8) 
        {
            mErrorCode = eErrorCode_DecodeError;
            return false;
        }

        if (mFilePosition + raw > mFileData.size()) 
        {
            mErrorCode = eErrorCode_DecodeError;
            return false;
        }

        outResult.mDataPtr = mFileData.data() + mFilePosition + 8;
        outResult.mLength = raw - 8;

        mFilePosition += raw;

        const bool tqi = (outResult.mTag == 0x54514970u); // 'pIQT' as LE
        const bool tgq = (outResult.mTag == 0x73514754u) || (outResult.mTag == 0x54475170u); // 'TGQs' / 'pQGT'
        if (tqi || tgq) 
        {
            mCodec = tqi ? eCodec::eCodec_Tqi : eCodec::eCodec_Tgq;
            return true;
        }

        // unknown: skip
        // todo: audio
    }
    return false;
}

void DK2MovieDecoder::YUV2RGB(ByteArray& outFrame) const 
{
    outFrame.resize(mFrameDims.x * mFrameDims.y * 3);

    const uint32_t ys = (uint32_t)mFrameDimsCoded.x;
    const uint32_t cs = (uint32_t)mFrameDimsCoded.x >> 1;

    for (int y = 0; y < mFrameDims.y; y++) 
    {
        const int cy = y >> 1;
        uint8_t* outBufferPtr = outFrame.data() + y * mFrameDims.x * 3;

        const uint8_t* y_data  = mY.data()  + y  * ys;
        const uint8_t* cb_data = mCb.data() + cy * cs;
        const uint8_t* cr_data = mCr.data() + cy * cs;

        for (int x = 0; x < mFrameDims.x; x++) 
        {
            const uint32_t cx = (uint32_t)x >> 1;

#if 0
            const int Y  = (int) y_data[x];
            const int Cb = (int) cb_data[cx] - 128;
            const int Cr = (int) cr_data[cx] - 128;

            // JPEG / full-range YCbCr
            const int r = Y + ((359 * Cr) >> 8);
            const int g = Y - ((88 * Cb + 183 * Cr) >> 8);
            const int b = Y + ((454 * Cb) >> 8);
#else
            // better looking contrast

            const int Y  = ((int)  y_data[x] - 16) * 298;         // 1.164 * 256
            const int Cb = (int) cb_data[cx] - 128;
            const int Cr = (int) cr_data[cx] - 128;

            const int r = (Y + 409 * Cr + 128) >> 8;            // 1.596 * 256
            const int g = (Y - 100 * Cb - 208 * Cr + 128) >> 8; // 0.392, 0.813
            const int b = (Y + 516 * Cb + 128) >> 8;            // 2.017 * 256
#endif

            outBufferPtr[x * 3 + 0] = clip8(r);
            outBufferPtr[x * 3 + 1] = clip8(g);
            outBufferPtr[x * 3 + 2] = clip8(b);
        }
    }
}

bool DK2MovieDecoder::DecodeTqi(const uint8_t* payload, uint32_t size, ByteArray& outFrame) 
{
    if (size < 8) 
    {
        mErrorCode = eErrorCode_FrameTooSmall;
        return false;
    }

    const int w = (int)rl16(payload + 0);
    const int h = (int)rl16(payload + 2);
    const int quant = payload[4];
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096) 
    {
        mErrorCode = eErrorCode_InvalidFrameDimensions;
        return false;
    }

    mFrameDims = {w, h};
    mFrameDimsCoded.x = align16(w);
    mFrameDimsCoded.y = align16(h);

    mY.assign((uint32_t)mFrameDimsCoded.x * mFrameDimsCoded.y, 0x80);
    mCb.assign((uint32_t)(mFrameDimsCoded.x >> 1) * (mFrameDimsCoded.y >> 1), 0x80);
    mCr.assign((uint32_t)(mFrameDimsCoded.x >> 1) * (mFrameDimsCoded.y >> 1), 0x80);

    uint16_t matrix[64];
    tqi_qtable(matrix, quant);

    BitMsb bits(payload + 8, size - 8);

    int last_dc[3] = {0, 0, 0};

    int16_t block[6][64];

    const int mb_w = mFrameDimsCoded.x / 16;
    const int mb_h = mFrameDimsCoded.y / 16;

    for (int my = 0; my < mb_h; my++) 
    {
        for (int mx = 0; mx < mb_w; mx++) 
        {
            for (int n = 0; n < 6; n++) 
            {
                if (!tqi_block(bits, block[n], n, last_dc, matrix)) 
                {
                    mErrorCode = eErrorCode_DecodeError;
                    return false;
                }
            }

            uint8_t* yp = mY.data() + (uint32_t)my * 16 * mFrameDimsCoded.x + mx * 16;
            uint8_t* cbp = mCb.data() + (uint32_t)my * 8 * (mFrameDimsCoded.x >> 1) + mx * 8;
            uint8_t* crp = mCr.data() + (uint32_t)my * 8 * (mFrameDimsCoded.x >> 1) + mx * 8;
            idct_put_mb(yp, mFrameDimsCoded.x, cbp, crp, mFrameDimsCoded.x >> 1, block);
        }
    }
    YUV2RGB(outFrame);
    return true;
}

bool DK2MovieDecoder::DecodeTgq(const uint8_t* payload, uint32_t size, ByteArray& outFrame) 
{
    if (size < 8) 
    {
        mErrorCode = eErrorCode_FrameTooSmall;
        return false;
    }

    const int w = (int)rl16(payload + 0);
    const int h = (int)rl16(payload + 2);

    const int quant = payload[4];
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096) 
    {
        mErrorCode = eErrorCode_InvalidFrameDimensions;
        return false;
    }

    mFrameDims = {w, h};

    mFrameDimsCoded.x = align16(w);
    mFrameDimsCoded.y = align16(h);

    const uint32_t ysz = (uint32_t) mFrameDimsCoded.x * mFrameDimsCoded.y;
    const uint32_t csz = (uint32_t)(mFrameDimsCoded.x >> 1) * (mFrameDimsCoded.y >> 1);

    mY.assign(ysz, 0x80);
    mCb.assign(csz, 0x80);
    mCr.assign(csz, 0x80);

    int qtable[64];
    tgq_qtable(qtable, quant);

    const uint8_t* p = payload + 8;

    uint32_t left = size - 8;
    int16_t block[6][64];

    const int mb_w = mFrameDimsCoded.x / 16;
    const int mb_h = mFrameDimsCoded.y / 16;

    for (int my = 0; my < mb_h; my++) 
    {
        for (int mx = 0; mx < mb_w; mx++) 
        {
            if (left < 1) 
            {
                mErrorCode = eErrorCode_DecodeError;
                return false;
            }

            const int mode = *p++;

            left--;
            uint8_t* yp = mY.data() + (uint32_t)my * 16 * mFrameDimsCoded.x + mx * 16;
            uint8_t* cbp = mCb.data() + (uint32_t)my * 8 * (mFrameDimsCoded.x >> 1) + mx * 8;
            uint8_t* crp = mCr.data() + (uint32_t)my * 8 * (mFrameDimsCoded.x >> 1) + mx * 8;

            if (mode > 12) 
            {
                if (left < (uint32_t)mode) 
                {
                    mErrorCode = eErrorCode_DecodeError;
                    return false;
                }

                BitLsb bits(p, (uint32_t)mode);
                for (int n = 0; n < 6; n++) 
                {
                    if (!tgq_block(bits, block[n], qtable)) 
                    {
                        mErrorCode = eErrorCode_DecodeError;
                        return false;
                    }
                }

                idct_put_mb(yp, mFrameDimsCoded.x, cbp, crp, mFrameDimsCoded.x >> 1, block);

                p += (uint32_t)mode;
                left -= (uint32_t)mode;
            } 
            else if (mode == 1) 
            {
                if (left < 1) 
                {
                    mErrorCode = eErrorCode_DecodeError;
                    return false;
                }

                const int mv = *p++;
                left--;
                int mv_x = mv >> 4;
                int mv_y = mv & 0x0F;

                if (mv_x >= 8) mv_x -= 16;
                if (mv_y >= 8) mv_y -= 16;

                const int x = mx * 16 - mv_x;
                const int y = my * 16 - mv_y;

                if (!mHaveReference || 
                    (x < 0) || (x + 16 > mFrameDimsCoded.x) || 
                    (y < 0) || (y + 16 > mFrameDimsCoded.y)) 
                {
                    // no ref / OOB: leave mid-gray
                } 
                else 
                {
                    copy16(yp, mFrameDimsCoded.x, mLastY.data() + (uint32_t)y * mFrameDimsCoded.x + x, mFrameDimsCoded.x, 16, 16);
                    copy16(cbp, mFrameDimsCoded.x >> 1,
                           mLastCb.data() + (uint32_t)(y >> 1) * (mFrameDimsCoded.x >> 1) + (x >> 1),
                           mFrameDimsCoded.x >> 1, 8, 8);
                    copy16(crp, mFrameDimsCoded.x >> 1,
                           mLastCr.data() + (uint32_t)(y >> 1) * (mFrameDimsCoded.x >> 1) + (x >> 1),
                        mFrameDimsCoded.x >> 1, 8, 8);
                }
            } 
            else 
            {
                int8_t dc[6] = {0, 0, 0, 0, 0, 0};
                if (mode == 3) 
                {
                    if (left < 3) 
                    { 
                        mErrorCode = eErrorCode_DecodeError;
                        return false; 
                    }

                    const int8_t ydc = s8(*p++);
                    dc[0] = dc[1] = dc[2] = dc[3] = ydc;
                    dc[4] = s8(*p++);
                    dc[5] = s8(*p++);
                    left -= 3;
                } 
                else if (mode == 6) 
                {
                    if (left < 6) 
                    {
                        mErrorCode = eErrorCode_DecodeError;
                        return false; 
                    }

                    for (int i = 0; i < 6; i++) 
                    {
                        dc[i] = s8(*p++);
                    }
                    left -= 6;
                } 
                else if (mode == 12) 
                {
                    if (left < 12) 
                    { 
                        mErrorCode = eErrorCode_DecodeError;
                        return false; 
                    }

                    for (int i = 0; i < 6; i++) 
                    {
                        dc[i] = s8(*p++);
                        p++;
                    }
                    left -= 12;
                } 
                else 
                {
                    mErrorCode = eErrorCode_DecodeError;
                    return false; 
                }

                auto dconly = [&](uint8_t* dst, int stride, int dcv) 
                    {
                        const int level = clip8((dcv * qtable[0] + 2056) >> 4);
                        fill8(dst, stride, (uint8_t)level);
                    };
                dconly(yp, mFrameDimsCoded.x, dc[0]);
                dconly(yp + 8, mFrameDimsCoded.x, dc[1]);
                dconly(yp + 8 * mFrameDimsCoded.x, mFrameDimsCoded.x, dc[2]);
                dconly(yp + 8 * mFrameDimsCoded.x + 8, mFrameDimsCoded.x, dc[3]);
                dconly(cbp, mFrameDimsCoded.x >> 1, dc[4]);
                dconly(crp, mFrameDimsCoded.x >> 1, dc[5]);
            }
        }
    }

    mLastY = mY;
    mLastCb = mCb;
    mLastCr = mCr;
    mHaveReference = true;
    YUV2RGB(outFrame);
    return true;
}

bool DK2MovieDecoder::DecodeNextFrame(ByteArray& outFrame) 
{
    DemuxResult demuxNextResult;
    if (!DemuxNext(demuxNextResult)) 
    {
        if (mErrorCode == eErrorCode_None)
        {
            mErrorCode = eErrorCode_EOF;
        }
        return false;
    }

    bool isSuccess = false;
    if (mCodec == eCodec::eCodec_Tqi)
    {
        isSuccess = DecodeTqi(demuxNextResult.mDataPtr, demuxNextResult.mLength, outFrame);
    }
    else
    {
        isSuccess = DecodeTgq(demuxNextResult.mDataPtr, demuxNextResult.mLength, outFrame);
    }
    if (isSuccess) 
    {
        mFrameIndex++;
    }
    return isSuccess;
}
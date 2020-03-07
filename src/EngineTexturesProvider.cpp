#include "pch.h"
#include "EngineTexturesProvider.h"
#include "Texture2D_Image.h"
#include "Console.h"
#include "FileSystem.h"

//////////////////////////////////////////////////////////////////////////

#define MAKEDWORD(ch0, ch1, ch2, ch3)                              \
                ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
                ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))

//////////////////////////////////////////////////////////////////////////

inline int clamp(int n, int min, int max)
{
    if (n < min) return min;
    if (n > max) return max;
    return n;
}

inline unsigned int round_up(double thearg)
{
    return (unsigned int)(thearg + (thearg > 0.0 ? 0.5 : -0.5));
}

//////////////////////////////////////////////////////////////////////////

const unsigned int TCHC_SIGNATURE = MAKEDWORD('T','C','H','C');

struct EngineTexturesDirHeader
{
public:
    unsigned int mSignature = 0; // TCHC
    int mDataLength = 0;
    int mVersion = 0;
    int mEntriesCount = 0;
};

//////////////////////////////////////////////////////////////////////////


/* external buffers and data to be supplied with sizes */
static const unsigned int magic_input_table_6c10c0[64] =
{
    0x2000, 0x1712, 0x187E, 0x1B37, 0x2000, 0x28BA, 0x3B21, 0x73FC, 0x1712,
    0x10A2, 0x11A8, 0x139F, 0x1712, 0x1D5D, 0x2AA1, 0x539F, 0x187E, 0x11A8,
    0x12BF, 0x14D4, 0x187E, 0x1F2C, 0x2D41, 0x58C5, 0x1B37, 0x139F, 0x14D4,
    0x1725, 0x1B37, 0x22A3, 0x3249, 0x62A3, 0x2000, 0x1712, 0x187E, 0x1B37,
    0x2000, 0x28BA, 0x3B21, 0x73FC, 0x28BA, 0x1D5D, 0x1F2C, 0x22A3, 0x28BA,
    0x33D6, 0x4B42, 0x939F, 0x3B21, 0x2AA1, 0x2D41, 0x3249, 0x3B21, 0x4B42,
    0x6D41, 0x0D650, 0x73FC, 0x539F, 0x58C5, 0x62A3, 0x73FC, 0x939F, 0x0D650,
    0x1A463
};

static const int magic_output_table[64] = // magic values computed from magic input
{ 

    //int d = (magic_input_table_6c10c0[i] & 0xfffe0000) >> 3;
    //int a = (magic_input_table_6c10c0[i] & 0x0001ffff) << 3;
    //magic_output_table[i] = d + a;

    65536, 47248, 50160, 55736, 65536, 83408, 121096, 237536, 47248, 34064, 
    36160, 40184, 47248, 60136, 87304, 171256, 50160, 36160, 38392, 42656, 
    50160, 63840, 92680, 181800, 55736, 40184, 42656, 47400, 55736, 70936, 
    102984, 202008, 65536, 47248, 50160, 55736, 65536, 83408, 121096, 237536, 
    83408, 60136, 63840, 70936, 83408, 106160, 154128, 302328, 121096, 87304, 
    92680, 102984, 121096, 154128, 223752, 438912, 237536, 171256, 181800, 202008, 
    237536, 302328, 438912, 860952
};

static const unsigned char jump_table_7af4e0[256] = {0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x66, 0x66, 0x66, 0x66, 0x77,
    0x77, 0x88, 0x0};

static const unsigned int dc_control_table_7af0e0[224] =
{
    0x00000000, 0x0000003f, 0x00000037, 0x0000003e,
    0x0000003d, 0x00000036, 0x0000002f, 0x00000027,
    0x0000002e, 0x00000035, 0x0000003c, 0x0000003b,
    0x00000034, 0x0000002d, 0x00000026, 0x0000001f,
    0x00000017, 0x0000001e, 0x00000025, 0x0000002c,
    0x00000033, 0x0000003a, 0x00000039, 0x00000032,
    0x0000002b, 0x00000024, 0x0000001d, 0x00000016,
    0x0000000f, 0x00000007, 0x0000000e, 0x00000015,
    0x0000001c, 0x00000023, 0x0000002a, 0x00000031,
    0x00000038, 0x00000030, 0x00000029, 0x00000022,
    0x0000001b, 0x00000014, 0x0000000d, 0x00000006,
    0x00000005, 0x0000000c, 0x00000013, 0x0000001a,
    0x00000021, 0x00000028, 0x00000020, 0x00000019,
    0x00000012, 0x0000000b, 0x00000004, 0x00000003,
    0x0000000a, 0x00000011, 0x00000018, 0x00000010,
    /* 60 */
    0x00000009, 0x00000002, 0x00000001, 0x00000008,
    0x00040102, 0x00040301, 0x00030201, 0x00030201,
    0x00024100, 0x00024100, 0x00024100, 0x00024100,
    /* 72 */
    0x00020101, 0x00020101, 0x00020101, 0x00020101,
    0x00064200, 0x00064200, 0x00064200, 0x00064200,
    0x00070302, 0x00070302, 0x00070a01, 0x00070a01,
    0x00070104, 0x00070104, 0x00070901, 0x00070901,
    0x00060801, 0x00060801, 0x00060801, 0x00060801,
    0x00060701, 0x00060701, 0x00060701, 0x00060701,
    0x00060202, 0x00060202, 0x00060202, 0x00060202,
    0x00060601, 0x00060601, 0x00060601, 0x00060601,
    0x00080e01, 0x00080106, 0x00080d01, 0x00080c01,
    0x00080402, 0x00080203, 0x00080105, 0x00080b01,
    0x00050103, 0x00050103, 0x00050103, 0x00050103,
    0x00050103, 0x00050103, 0x00050103, 0x00050103,
    0x00050501, 0x00050501, 0x00050501, 0x00050501,
    0x00050501, 0x00050501, 0x00050501, 0x00050501,
    /* 128 */
    0x00050401, 0x00050401, 0x00050401, 0x00050401,
    0x00050401, 0x00050401, 0x00050401, 0x00050401,
    0x000a1101, 0x000a0602, 0x000a0107, 0x000a0303,
    0x000a0204, 0x000a1001, 0x000a0f01, 0x000a0502,
    /* 144 */
    0x000c010b, 0x000c0902, 0x000c0503, 0x000c010a,
    0x000c0304, 0x000c0802, 0x000c1601, 0x000c1501,
    0x000c0109, 0x000c1401, 0x000c1301, 0x000c0205,
    0x000c0403, 0x000c0108, 0x000c0702, 0x000c1201,
    /* 160 */
    0x000d0b02, 0x000d0a02, 0x000d0603, 0x000d0404,
    0x000d0305, 0x000d0207, 0x000d0206, 0x000d010f,
    0x000d010e, 0x000d010d, 0x000d010c, 0x000d1b01,
    0x000d1a01, 0x000d1901, 0x000d1801, 0x000d1701,
    /* 176 */
    0x000e011f, 0x000e011e, 0x000e011d, 0x000e011c,
    0x000e011b, 0x000e011a, 0x000e0119, 0x000e0118,
    0x000e0117, 0x000e0116, 0x000e0115, 0x000e0114,
    0x000e0113, 0x000e0112, 0x000e0111, 0x000e0110,
    /* 192 */
    0x000f0128, 0x000f0127, 0x000f0126, 0x000f0125,
    0x000f0124, 0x000f0123, 0x000f0122, 0x000f0121,
    0x000f0120, 0x000f020e, 0x000f020d, 0x000f020c,
    0x000f020b, 0x000f020a, 0x000f0209, 0x000f0208,
    0x00100212, 0x00100211, 0x00100210, 0x0010020f,
    0x00100703, 0x00101102, 0x00101002, 0x00100f02,
    0x00100e02, 0x00100d02, 0x00100c02, 0x00102001,
    0x00101f01, 0x00101e01, 0x00101d01, 0x00101c01
};

static const float float_7af000 = 1.048576e6f;
static const float float_7af004 = 8.388608e6f;
static const float float_7af008 = 1.169f;
static const float float_7af00c = -8.1300002e-1f;
static const float float_7af010 = -3.91e-1f;
static const float float_7af014 = 1.602f;
static const float float_7af018 = 2.0250001f;
static const double double_7af048 = 6.75539944108852e15;
static const unsigned int norm_7af038 = 0x5A82799A;
static const float float_7af03c = 5.4119611e-1f;
static const float float_7af040 = 1.306563f;
static const float float_7af044 = 3.8268343e-1f;

//////////////////////////////////////////////////////////////////////////

class stream_uint32
{
public:
    stream_uint32(FILE* fileStream, int theOrigin, int theLength)
        : mStream(fileStream)
        , mOrigin(theOrigin)
        , mLength(theLength)
        , mPageIndex()
        , mCacheSize()
    {
        refill_cache();
    }

    // read uint from specific location
    inline unsigned int get(unsigned int theOffset)
    {
        const int requestBufferPage = theOffset / NUM_CACHE_ELEMENTS;
        if (requestBufferPage != mPageIndex)
        {
            mPageIndex = requestBufferPage;
            refill_cache();
        }
        const int ielement = theOffset % NUM_CACHE_ELEMENTS;
        return (ielement < mCacheSize) ? mCache[ielement] : 0;
    }

private:
    enum { NUM_CACHE_ELEMENTS = 256 };

    enum { Sizeof_CacheElement = sizeof(unsigned int) };
    enum { Sizeof_Cache = NUM_CACHE_ELEMENTS * Sizeof_CacheElement };
    // cache data from file stream
    inline void refill_cache()
    {
        if (!cxx::set_filepos(mStream, mOrigin + mPageIndex * Sizeof_Cache))
        {
            debug_assert(false);
            return;
        }
        mCacheSize = ::fread(mCache, Sizeof_CacheElement, NUM_CACHE_ELEMENTS, mStream);
    }

private:
    FILE* mStream = nullptr;
    int mOrigin;
    int mLength;
    int mPageIndex;
    int mCache[512];
    int mCacheSize; // in uints
};

static stream_uint32* bs;
static unsigned int bs_index;
static unsigned int bs_red = 0;
static unsigned int bs_green = 0;
static unsigned int bs_blue = 0;
static unsigned int bs_alpha = 0;

static int decompress2_chunk[256]; /* buffers */
static int decompress3_chunk[288];
static int decompress4_chunk[512];

inline unsigned int bs_read(unsigned int pos, int bits)
{
    unsigned int w1, w2;
    unsigned int word_index;
    unsigned int shamt;

    word_index = pos >> 5;
    shamt = pos & 0x1f;
    w1 = bs->get(word_index) << shamt;
    w2 = shamt ? bs->get(word_index + 1) >> (32 - shamt) : 0;
    w1 |= w2;
    w1 >>= (32 - bits);

    return w1;
}

static unsigned int prepare_decompress(unsigned int value, unsigned int pos)
{
    int xindex, index, control_word = 0;
    unsigned char magic_index = 0x3f;

    decompress2_chunk[0] = value * magic_output_table[0];
    memset(&decompress2_chunk[1], 0,
        sizeof(decompress2_chunk) - sizeof(unsigned int));

    for (;;)
    {
        xindex = index = bs_read(pos, 17);
        if (index >= 0x8000) 
        {
            index >>= 13;
            control_word = dc_control_table_7af0e0[60 + index];
        } 
        else if (index >= 0x800) 
        {
            index >>= 9;
            control_word = dc_control_table_7af0e0[72 + index];
        } 
        else if (index >= 0x400) 
        {
            index >>= 7;
            control_word = dc_control_table_7af0e0[128 + index];
        } 
        else if (index >= 0x200) 
        {
            index >>= 5;
            control_word = dc_control_table_7af0e0[128 + index];
        } 
        else if (index >= 0x100) 
        {
            index >>= 4;
            control_word = dc_control_table_7af0e0[144 + index];
        } 
        else if (index >= 0x80) 
        {
            index >>= 3;
            control_word = dc_control_table_7af0e0[160 + index];
        } 
        else if (index >= 0x40) 
        {
            index >>= 2;
            control_word = dc_control_table_7af0e0[176 + index];
        } 
        else if (index >= 0x20) 
        {
            index >>= 1;
            control_word = dc_control_table_7af0e0[192 + index];
        }

        if ((control_word & 0xff00) == 0x4100)
        {
            break;
        }

        if ((control_word & 0xff00) > 0x4100) 
        {
            unsigned short unk14;
            // read next control 
            pos += control_word >> 16;
            unk14 = bs_read(pos, 14);
            pos += 14;
            magic_index -= (unk14 & 0xff00) >> 8;
            unk14 &= 0xff;
            if (unk14) 
            {
                if (unk14 != 0x80) 
                {
                    if(unk14 > 0x80) 
                        unk14 -= 0x100;
                    magic_index--;
                } 
                else 
                {
                    unk14 = bs_read(pos, 8);
                    pos += 8;
                    unk14 -= 0x100;
                }
            } 
            else 
            {
                unk14 = bs_read(pos, 8);
                pos += 8;
            }
            control_word = unk14;
        } 
        else 
        {
            int bit_to_test;
            unsigned int rem = control_word >> 16;
            unsigned int xoramt = 0;
            magic_index -= (control_word & 0xff00) >> 8;
            bit_to_test = 16 - rem;
            if (xindex & (1 << bit_to_test))
                xoramt = ~0;
            control_word &= 0xff;
            control_word ^= xoramt;
            pos++;
            control_word -= xoramt;
            pos += rem;
        }

        unsigned int out_index = dc_control_table_7af0e0[magic_index + 1];
        decompress2_chunk[out_index] = ((short) control_word) * magic_output_table[out_index];
    }
    return pos + (control_word >> 16);
}

static void decompress_func1(int *in, int *out)
{
    int64_t rx;
    int sa;
    int b, a, c, d, i, p, s;
    unsigned int sc, sd, si;
    double rxf, rxg, rxs;
    double xf, xg;

    if(!(in[1] | in[2] | in[3] | in[4] | in[6] | in[7])) 
    {
        a = in[0];
        out[0] = a;
        out[9] = a;
        out[18] = a;
        out[27] = a;
        out[36] = a;
        out[45] = a;
        out[54] = a;
        out[63] = a;
        return;
    }

    b = in[5] - in[3];
    c = in[1] - in[7];
    i = in[3] + in[5];
    a = in[7] + in[1];
    xf = b;
    xg = c;
    p = i + a;
    a -= i;

    rxs = xg + xf;
    rxf = xf * float_7af03c + float_7af044 * rxs;
    rxg = xg * float_7af040 - float_7af044 * rxs;
    int ra = (int)(rxf + (rxf > 0 ? 0.5f : -0.5f));
    int rb = (int)(rxg + (rxg > 0 ? 0.5f : -0.5f));

    sa = a;
    rx = sa;
    rx *= norm_7af038;
    a = (int)rx;
    d = rx >> 32;

    b = in[6];
    d += d;
    a = in[2];

    c = ra;
    i = rb;
    c += d;
    d += i;
    i += p;
    sc = c;
    sd = d;
    si = i;
    c = in[0];
    d = in[4];
    s = b + a;
    a -= b;
    b = d + c;
    c -= d;

    sa = a;
    rx = sa;
    rx *= norm_7af038;
    a = (int)rx;
    d = rx >> 32;

    d += d;
    out[18] = (c - d) + sc;
    out[45] = (c - d) - sc;
    out[27] = (b - (s + d)) + ra;
    out[36] = (b - (s + d)) - ra;
    out[0] = (s + d) + b + si;
    out[9] = sd + d + c;
    out[54] = d + c - sd;
    out[63] = (s + d) + b - si;
}

static void decompress_func2(int *in, int *out)
{
    int64_t rx;
    int sa;
    int b, a, c, d, i, p, s;
    unsigned int sc, sd, si;
    int ra, rb;
    double rxf, rxg, rxs;
    double xf, xg;

    b = in[5] - in[3];
    c = in[1] - in[7];
    i = in[3] + in[5];
    a = in[7] + in[1];
    xf = b;
    xg = c;
    p = i + a;
    a -= i;

    rxs = xg + xf;
    rxf = xf * float_7af03c + float_7af044 * rxs;
    rxg = xg * float_7af040 - float_7af044 * rxs;
    ra = (int)(rxf + (rxf > 0 ? 0.5f : -0.5f));
    rb = (int)(rxg + (rxg > 0 ? 0.5f : -0.5f));

    sa = a;
    rx = sa;
    rx *= norm_7af038;
    a = (int)rx;
    d = rx >> 32;

    b = in[6];
    d += d;
    a = in[2];

    c = ra;
    i = rb;
    c += d;
    d += i;
    i += p;
    sc = c;
    sd = d;
    si = i;
    c = in[0];
    d = in[4];
    s = b + a;
    a -= b;
    b = d + c;
    c -= d;

    sa = a;
    rx = sa;
    rx *= norm_7af038;
    a = (int)rx;
    d = rx >> 32;

    d += d;
    p = sc;
    s += d;
    a = d + c;
    c -= d;
    d = s + b;
    b -= s;
    s = c + p;
    c -= p;
    p = ra;
    out[2] = s;
    s = sd;
    out[5] = c;
    c = b + p;
    b -= p;
    p = si;
    out[3] = c;
    out[4] = b;
    b = s + a;
    a -= s;
    c = d + p;
    d -= p;
    out[0] = c;
    out[1] = b;
    out[6] = a;
    out[7] = d;
}

static void decompress(bool hasAlpha)
{
    unsigned char jt_index, jt_value;
    unsigned int bs_pos = bs_index;
    int value;
    unsigned char blanket_fill;

    /* red */
    value = 0;
    jt_index = bs_read(bs_pos, 8);

    jt_value = jump_table_7af4e0[jt_index];
    bs_pos += jt_value & 0xf;
    jt_value >>= 4;
    if( jt_value ) {
        /* value is signed */
        value = bs_read(bs_pos, jt_value);
        if( (value & (1 << (jt_value - 1))) == 0 )
            value -= (1 << jt_value) - 1;

        bs_pos += jt_value;
    }

    bs_red += value;
    blanket_fill = bs_read(bs_pos, 2);
    if( blanket_fill == 2 ) {
        int i, j;
        bs_pos += 2;
        for( j = 0; j < 8; j++ )
            for( i = 0; i < 8; i++ )
                decompress4_chunk[j * 64 + i] = bs_red << 16;
        bs_index = bs_pos;
    } else {
        int i;
        bs_index = prepare_decompress(bs_red, bs_pos);
        for( i = 0; i < 8; i++ )
            decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
        for( i = 0; i < 8; i++ )
            decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64]);
    }

    bs_pos = bs_index;

    /* green */
    value = 0;
    jt_index = bs_read(bs_pos, 8);

    jt_value = jump_table_7af4e0[jt_index];
    bs_pos += jt_value & 0xf;
    jt_value >>= 4;
    if( jt_value ) {
        /* value is signed */
        value = bs_read(bs_pos, jt_value);
        if( (value & (1 << (jt_value - 1))) == 0 )
            value -= (1 << jt_value) - 1;

        bs_pos += jt_value;
    }

    bs_green += value;
    blanket_fill = bs_read(bs_pos, 2);
    if( blanket_fill == 2 ) {
        int i, j;
        bs_pos += 2;
        for( j = 0; j < 8; j++ )
            for( i = 0; i < 8; i++ )
                decompress4_chunk[j * 64 + i + 9] = bs_green << 16;
        bs_index = bs_pos;
    } else {
        int i;
        bs_index = prepare_decompress(bs_green, bs_pos);
        for( i = 0; i < 8; i++ )
            decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
        for( i = 0; i < 8; i++ )
            decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64 + 9]);
    }

    bs_pos = bs_index;

    /* blue */
    value = 0;
    jt_index = bs_read(bs_pos, 8);

    jt_value = jump_table_7af4e0[jt_index];
    bs_pos += jt_value & 0xf;
    jt_value >>= 4;
    if( jt_value ) {
        /* value is signed */
        value = bs_read(bs_pos, jt_value);
        if( (value & (1 << (jt_value - 1))) == 0 )
            value -= (1 << jt_value) - 1;

        bs_pos += jt_value;
    }

    bs_blue += value;
    blanket_fill = bs_read(bs_pos, 2);
    if( blanket_fill == 2 ) {
        int i, j;
        bs_pos += 2;
        for( j = 0; j < 8; j++ )
            for( i = 0; i < 8; i++ )
                decompress4_chunk[j * 64 + i + 18] = bs_blue << 16;
        bs_index = bs_pos;
    } else {
        int i;
        bs_index = prepare_decompress(bs_blue, bs_pos);
        for( i = 0; i < 8; i++ )
            decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
        for( i = 0; i < 8; i++ )
            decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64 + 18]);
    }

    bs_pos = bs_index;

    /* alpha */
    if(!hasAlpha) return;

    value = 0;
    jt_index = bs_read(bs_pos, 8);

    jt_value = jump_table_7af4e0[jt_index];
    bs_pos += jt_value & 0xf;
    jt_value >>= 4;
    if( jt_value ) {
        /* value is signed */
        value = bs_read(bs_pos, jt_value);
        if( (value & (1 << (jt_value - 1))) == 0 )
            value -= (1 << jt_value) - 1;

        bs_pos += jt_value;
    }

    bs_alpha += value;
    blanket_fill = bs_read(bs_pos, 2);
    if( blanket_fill == 2 ) {
        int i, j;
        bs_pos += 2;
        for( j = 0; j < 8; j++ )
            for( i = 0; i < 8; i++ )
                decompress4_chunk[j * 64 + i + 27] = bs_alpha << 16;
        bs_index = bs_pos;
    } else {
        int i;
        bs_index = prepare_decompress(bs_alpha, bs_pos);
        for( i = 0; i < 8; i++ )
            decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
        for( i = 0; i < 8; i++ )
            decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64 + 27]);
    }
}

static void decompress_block(unsigned char *out, unsigned short stride, bool hasAlpha)
{
    decompress(hasAlpha);

    const int* inp = decompress4_chunk;
    for (int j = 0; j < 8; ++j) 
    {
        for(int i = 0; i < 8; ++i) 
        {
            float r = inp[i +  0] * 1.0f;
            float g = inp[i + 18] * 1.0f;
            float b = inp[i +  9] * 1.0f;
            int a = inp[i + 27];
            int ir = round_up(float_7af014 * (g - float_7af004) + float_7af008 * (r - float_7af000) + double_7af048);
            int ig = round_up(float_7af018 * (b - float_7af004) + float_7af008 * (r - float_7af000) + double_7af048);
            int ib = round_up(float_7af010 * (b - float_7af004) + float_7af00c * (g - float_7af004) + float_7af008 * (r - float_7af000) + double_7af048);
            out[i * 4 + 0] = clamp(ir >> 16, 0, 255);
            out[i * 4 + 2] = clamp(ig >> 16, 0, 255);
            out[i * 4 + 1] = clamp(ib >> 16, 0, 255);
            out[i * 4 + 3] = hasAlpha ? clamp(a >> 16, 0, 255) : 255;
        }
        out += stride;
        inp += 64;
    }
}

inline void initialize_dd(stream_uint32 *buf)
{
    bs = buf;
    bs_index = 0;
    bs_red = 0;
    bs_blue = 0;
    bs_green = 0;
    bs_alpha = 0;
}

static void dd_texture(stream_uint32 *buf, unsigned char *outp, unsigned int stride, unsigned short width, unsigned short height, bool hasAlpha)
{
    initialize_dd(buf);

    for(unsigned short y = 0; y < height; y += 8)
        for(unsigned short x = 0; x < width; x += 8) 
        {
            decompress_block(&outp[y * stride + x * 4], stride, hasAlpha);
        }
}

//////////////////////////////////////////////////////////////////////////

inline int GetMipmapFromName(const std::string& input_string)
{
    const int stringLength = input_string.length();
    if (stringLength < 3)
    {
        return -1;
    }
    bool isMipmap = (input_string[stringLength - 3] == 'M') && (input_string[stringLength - 2] == 'M');
    if (!isMipmap)
    {
        return -1;
    }

    char mchar = input_string[stringLength - 1];
    if (mchar < '0' || mchar > '9')
    {
        return -1;
    }
    return mchar - '0';
}

//////////////////////////////////////////////////////////////////////////

EngineTexturesProvider gEngineTexturesProvider;

EngineTexturesProvider::EngineTexturesProvider()
{
}

EngineTexturesProvider::~EngineTexturesProvider()
{
}

bool EngineTexturesProvider::Initialize()
{
    debug_assert(mFileStream == nullptr);

    if (gFileSystem.mDungeonKeeperGameTextureCacheDirPath.empty())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Game engines textures cache path not specified");
        return false;
    }

    fs::path engineTexturesDir {gFileSystem.mDungeonKeeperGameTextureCacheDirPath};
    fs::path datFilepath = engineTexturesDir / "EngineTextures.dat";
    fs::path dirFilepath = engineTexturesDir / "EngineTextures.dir";

    if (!fs::exists(datFilepath) || !fs::is_regular_file(datFilepath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot locate file 'EngineTextures.dat'");
        return false;
    }

    if (!fs::exists(dirFilepath) || !fs::is_regular_file(dirFilepath))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot locate file 'EngineTextures.dir'");
        return false;
    }

    if (!ParseDirContent(dirFilepath.generic_string(), datFilepath.generic_string()))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read EngineTextures content");
        return false;
    }

    return true;
}

void EngineTexturesProvider::Deinit()
{
    if (mFileStream)
    {
        ::fclose(mFileStream);
        mFileStream = nullptr;
    }
    mEntiesArray.clear();
    mIndicesMap.clear();
}

bool EngineTexturesProvider::ContainsTexture(const std::string& textureName) const
{
    auto find_iterator = mIndicesMap.find(textureName);
    if (find_iterator == mIndicesMap.end())
        return false;

    return true;
}

bool EngineTexturesProvider::ExtractTexture(const std::string& textureName, Texture2D_Image& imageData) const
{
    auto find_iterator = mIndicesMap.find(textureName);
    if (find_iterator == mIndicesMap.end())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Engine texture does not exists '%s'", textureName.c_str());
        return false;
    }

    const TextureEntryIndex& entryIndex = find_iterator->second;

    const int linearIndex = entryIndex.mMipIndices[0];
    debug_assert(linearIndex != -1);
    const TextureEntryStruct& entryStruct = mEntiesArray[linearIndex];

    // extract texture with all mipmaps
    Size2D textureDimensions { entryStruct.mSizeX, entryStruct.mSizeY };
    imageData.CreateImage(eTextureFormat_RGBA8, textureDimensions, entryIndex.mNumImages - 1, entryStruct.mHasAlpha);
    for (int icurr = 0; icurr < entryIndex.mNumImages; ++icurr)
    {
        unsigned char* dataBuffer = imageData.GetImageDataBuffer(icurr);
        if (icurr == 0)
        {
            if (!ExtractTexturePixels(linearIndex, dataBuffer))
            {
                debug_assert(false);
                gConsole.LogMessage(eLogMessage_Warning, "Cannot extract texture data '%s'", textureName.c_str());
                return false;
            }
        }
        else
        {
            const int mipLinearIndex = entryIndex.mMipIndices[icurr];
            debug_assert(mipLinearIndex != -1);

            const TextureEntryStruct& mipmapStruct = mEntiesArray[mipLinearIndex];
            debug_assert(mipmapStruct.mSizeX == GetTextureMipmapDims(textureDimensions.x, icurr));
            debug_assert(mipmapStruct.mSizeY == GetTextureMipmapDims(textureDimensions.y, icurr));

            if (!ExtractTexturePixels(mipLinearIndex, dataBuffer))
            {
                debug_assert(false);
                gConsole.LogMessage(eLogMessage_Warning, "Cannot extract texture mipmap data '%s'", textureName.c_str());
            }
        }
    }
    return true;
}

void EngineTexturesProvider::DumpTextures(const std::string& outputDirectory) const
{
    fs::path outDirectoryPath { outputDirectory };
    if (!fs::is_directory(outDirectoryPath))
    {
        if (!fs::create_directories(outDirectoryPath))
        {
            debug_assert(false);
            return;
        }
    }

    Texture2D_Image imageData;
    for (const auto& indices_iterator: mIndicesMap)
    {
        imageData.Clear();

        const TextureEntryIndex& entryIndex = indices_iterator.second;
        debug_assert(entryIndex.mNumImages > 0);

        const int primaryIndex = entryIndex.mMipIndices[0];
        debug_assert(primaryIndex != -1);
        const TextureEntryStruct& primaryEntry = mEntiesArray[primaryIndex];

        Size2D textureDimensions { primaryEntry.mSizeX, primaryEntry.mSizeY };
        imageData.CreateImage(eTextureFormat_RGBA8, textureDimensions, 0, primaryEntry.mHasAlpha);

        unsigned char* dataBuffer = imageData.GetImageDataBuffer();
        if (!ExtractTexturePixels(primaryIndex, dataBuffer))
        {
            debug_assert(false);
            gConsole.LogMessage(eLogMessage_Warning, "Cannot extract texture data '%s'", indices_iterator.first.c_str());
            continue;
        }

        std::string tempName = indices_iterator.first;
        tempName.append(".png");
        cxx::path_remove_forbidden_chars(tempName); // make sure path is good

        fs::path filePath = outDirectoryPath / fs::path { tempName };
        if (!imageData.DumpToFile(filePath.generic_string()))
        {
            debug_assert(false);
            gConsole.LogMessage(eLogMessage_Warning, "Cannot save texture '%s'", indices_iterator.first.c_str());
            continue;
        }
    }
}

bool EngineTexturesProvider::ParseDirContent(const std::string& dirFilepath, const std::string& datFilepath)
{
    ByteArray dirContent;
    EngineTexturesDirHeader dirHeader;

    {
        cxx::file_unique_ptr scope_file { dirFilepath.c_str(), "rb" };
        if (scope_file.mFileStream == nullptr)
        {
            debug_assert(false);
            return false;
        }

        // read header
        cxx::read_data(scope_file.mFileStream, &dirHeader, sizeof(EngineTexturesDirHeader));
        if (dirHeader.mSignature != TCHC_SIGNATURE)
        {
            debug_assert(false);
            return false;
        }

        // read dir content
        const int dirLength = dirHeader.mDataLength - sizeof(EngineTexturesDirHeader);
        dirContent.resize(dirLength);
        if (!cxx::read_data(scope_file.mFileStream, dirContent.data(), dirLength))
        {
            debug_assert(false);
            return false;
        }
    }

    // scan dir entities
    mFileStream = ::fopen(datFilepath.c_str(), "rb");
    if (mFileStream == nullptr)
    {
        debug_assert(false);
        return false;
    }

    // allocate entries
    mEntiesArray.resize(dirHeader.mEntriesCount);

    std::string nameString;

    // read entries
    unsigned char* cursorPos = dirContent.data();
    for (int i = 0; i < dirHeader.mEntriesCount; ++i)
    {
        nameString.assign((const char*)cursorPos); // read name
                                                   // and advance cursor

        cursorPos += nameString.length() + 1;

        int mipIndex = GetMipmapFromName(nameString);
        if (mipIndex < 0 || mipIndex > TEXTURE_MIP_LEVEL_COUNT - 1)
        {
            // do not add to index
            gConsole.LogMessage(eLogMessage_Debug, "Ignore texture '%s'", nameString.c_str());
        }
        else // add to index
        {
            // remove MMn suffix
            nameString.erase(nameString.begin() + nameString.length() - 3, nameString.end());

            TextureEntryIndex& entryIndex = mIndicesMap[nameString];
            debug_assert(entryIndex.mMipIndices[mipIndex] == -1);
            entryIndex.mMipIndices[mipIndex] = i;
            ++entryIndex.mNumImages;
        }

        TextureEntryStruct& entryStruct = mEntiesArray[i];
        // read data offset ot dat file
        int offset = cursorPos[0] | 
            (cursorPos[1] << 8) |
            (cursorPos[2] << 16) |
            (cursorPos[3] << 24);

        cursorPos += 4;

        // read dat header
        cxx::set_filepos(mFileStream, offset);

        cxx::read_int32(mFileStream, entryStruct.mSizeX);
        cxx::read_int32(mFileStream, entryStruct.mSizeY);
        cxx::read_int32(mFileStream, entryStruct.mDataLength);
        entryStruct.mDataLength = entryStruct.mDataLength - 8; // correct

        cxx::read_int16(mFileStream, entryStruct.mSizeX2);
        cxx::read_int16(mFileStream, entryStruct.mSizeY2);

        unsigned int textureFlags = 0;
        cxx::read_uint32(mFileStream, textureFlags);

        entryStruct.mHasAlpha = (textureFlags >> 7) > 0;
        entryStruct.mDataStartLocation = offset + 20;
    }
    gConsole.LogMessage(eLogMessage_Info, "Found %d unique entries in engine textures cache", mIndicesMap.size());
    return true;
}

bool EngineTexturesProvider::ExtractTexturePixels(int entryIndex, unsigned char* outputBuffer) const
{
    const TextureEntryStruct& entryStruct = mEntiesArray[entryIndex];

    stream_uint32 uint32stream (mFileStream, entryStruct.mDataStartLocation, entryStruct.mDataLength);
    dd_texture(&uint32stream, outputBuffer, 
        entryStruct.mSizeX * 4,
        entryStruct.mSizeX,
        entryStruct.mSizeY, entryStruct.mHasAlpha);
    return true;
}

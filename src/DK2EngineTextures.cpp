#include "stdafx.h"
#include "DK2EngineTextures.h"

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
    stream_uint32(std::istream& theStream, int theOrigin, int theLength)
        : mStream(theStream)
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
        const int requestBufferPage = theOffset / Cache_ElementsCount;
        if (requestBufferPage != mPageIndex)
        {
            mPageIndex = requestBufferPage;
            refill_cache();
        }
        const int ielement = theOffset % Cache_ElementsCount;
        return (ielement < mCacheSize) ? mCache[ielement] : 0;
    }

private:

    static const int Cache_ElementsCount = 256;
    static const int Cache_ElementSize = 4; // sizeof(uint)
    static const int Cache_SizeBytes = Cache_ElementsCount * Cache_ElementSize;

    // cache data from file stream
    inline void refill_cache()
    {
        int offsetFromOrigin = mPageIndex * Cache_SizeBytes;
        int offset = mOrigin + offsetFromOrigin;
        if (!mStream.seekg(offset, std::ios::beg))
        {
            cxx_assert(false);
            return;
        }

        int numElementsToRead = Cache_ElementsCount;

        if ((offsetFromOrigin + Cache_SizeBytes) > mLength)
        {
            int numAvailableBytes = (mLength - offsetFromOrigin);
            numElementsToRead = numAvailableBytes / Cache_ElementSize;

            cxx_assert((numAvailableBytes % Cache_ElementSize) == 0);
        }

        mCacheSize = cxx::read_some_elements(mStream, mCache, numElementsToRead);
        if (!mStream)
        {
            cxx_assert(false);
            return;
        }
    }

private:
    std::istream& mStream;
    int mOrigin;
    int mLength;
    int mPageIndex;
    int mCache[512];
    int mCacheSize; // in uints
};

//////////////////////////////////////////////////////////////////////////

stream_uint32* bs;
unsigned int bs_index;
unsigned int bs_red = 0;
unsigned int bs_green = 0;
unsigned int bs_blue = 0;
unsigned int bs_alpha = 0;

int decompress2_chunk[256]; /* buffers */
int decompress3_chunk[288];
int decompress4_chunk[512];

//////////////////////////////////////////////////////////////////////////

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

static void decompress_block_444(unsigned char *out, unsigned short stride, bool hasAlpha)
{
    unsigned int bs_pos = bs_index;
    unsigned int red = bs_read(bs_pos, 8);
    bs_index = prepare_decompress(red, bs_pos + 8);
    for (int i = 0; i < 8; i++) 
    {
        decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
    }
    for (int i = 0; i < 8; i++) 
    {
        decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64]);
    }
    bs_pos = bs_index;

    unsigned int green = bs_read(bs_pos, 8);
    bs_index = prepare_decompress(green, bs_pos + 8);
    for (int i = 0; i < 8; i++) 
    {
        decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
    }
    for (int i = 0; i < 8; i++) 
    {
        decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64 + 9]);
    }
    bs_pos = bs_index;

    unsigned int blue = bs_read(bs_pos, 8);
    bs_index = prepare_decompress(blue, bs_pos + 8);
    for (int i = 0; i < 8; i++) 
    {
        decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
    }
    for (int i = 0; i < 8; i++) 
    {
        decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64 + 18]);
    }
    bs_pos = bs_index;

    if (hasAlpha)
    {
        unsigned int alpha = bs_read(bs_pos, 8);
        bs_index = prepare_decompress(alpha, bs_pos + 8);
        for (int i = 0; i < 8; i++) 
        {
            decompress_func1(&decompress2_chunk[i * 8], &decompress3_chunk[i]);
        }
        for (int i = 0; i < 8; i++) 
        {
            decompress_func2(&decompress3_chunk[i * 9], &decompress4_chunk[i * 64 + 27]);
        }
        bs_pos = bs_index;
    }

    /* another check for a flag at 668dc7, set in the master routine */
    /* dword_7af600 = dest */

    const int* inp = decompress4_chunk;
    if (hasAlpha) 
    {
        cxx_assert(false);
    } 
    else 
    {
        const int* inp = decompress4_chunk;
        for (int j = 0; j < 8; ++j) 
        {
            for(int i = 0; i < 8; ++i) 
            {
                int r = inp[i +  0];
                int g = inp[i + 18];
                int b = inp[i +  9];

                out[i * 4 + 0] = clamp(r >> 16, 0, 255);
                out[i * 4 + 1] = clamp(b >> 16, 0, 255);
                out[i * 4 + 2] = clamp(g >> 16, 0, 255);
                out[i * 4 + 3] = 255;
            }
            out += stride;
            inp += 64;
        }
    }
}

static void dd_texture_444(stream_uint32 *buf, unsigned char *outp, unsigned int stride, unsigned short width, unsigned short height, bool hasAlpha)
{
    initialize_dd(buf);

    for(unsigned short y = 0; y < height; y += 8)
    {
        for(unsigned short x = 0; x < width; x += 8) 
        {
            decompress_block_444(&outp[y * stride + x * 4], stride, hasAlpha);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

struct DK2EngineTextureEntry
{
    DK2EngineTextureEntry()
        : mName()
        , mSizeX()
        , mSizeY()
        , mSizeX2()
        , mSizeY2()
        , mDataLength()
        , mDataStartLocation()
        , mHasAlpha()
    {}
    
    // data
    const char* mName;
    int mSizeX;
    int mSizeY;
    int mSizeX2;
    int mSizeY2;
    int mDataLength;
    int mDataStartLocation;
    bool mHasAlpha;
};

typedef std::vector<DK2EngineTextureEntry> DK2EngineTextureEntries;
struct DK2EngineTexturesCacheDir
{
public:
    std::ifstream           mDatStream;
    ByteArray               mDirContent;
    DK2EngineTextureEntries mEntries;
};
static DK2EngineTexturesCacheDir* gCacheDir = nullptr;

//////////////////////////////////////////////////////////////////////////
static bool dk2_read_dir_content(const char* theDirFilepath, const char* theDatFilename)
{

    struct dk2_dir_header
    {
        unsigned int mSignature; // TCHC
        int mDataLength;
        int mVersion;
        int mEntriesCount;
    };

    int numEntries = 0;
    { std::ifstream fileStream {theDirFilepath, std::ios::in | std::ios::binary};
        if (!fileStream.is_open())
            return false;

        // read header
        dk2_dir_header dirHeader {};
        cxx::read_elements(fileStream, &dirHeader, 1);
        if (dirHeader.mSignature != MAKEDWORD('T','C','H','C'))
            return false;

        // read dir content
        const int dirLength = dirHeader.mDataLength - sizeof(dk2_dir_header);
        gCacheDir->mDirContent.resize(dirLength);
        if(!fileStream.read((char*) &gCacheDir->mDirContent[0], dirLength))
            return false;

        numEntries = dirHeader.mEntriesCount;
    }

    // scan dir entities
    gCacheDir->mDatStream.open(theDatFilename, std::ios::in | std::ios::binary);
    if (!gCacheDir->mDatStream.is_open())
        return false;

    // allocate entries
    gCacheDir->mEntries.resize(numEntries);

    // read entries
    unsigned char* cursorPos = &gCacheDir->mDirContent[0];
    for (int i = 0; i < numEntries; ++i)
    {
        // read name
        gCacheDir->mEntries[i].mName = (const char*)cursorPos;
        for (;;) {
            if (*(cursorPos++) == 0)
                break;
        }
        // read data offset ot dat file
        int offset = cursorPos[0] | 
            (cursorPos[1] << 8) |
            (cursorPos[2] << 16) |
            (cursorPos[3] << 24);

        cursorPos += 4;

        // read dat header
        gCacheDir->mDatStream.seekg(offset, std::ios::beg);

        int sizeX = 0;
        int sizeY = 0;
        int dataSize = 0;
        cxx::read_elements(gCacheDir->mDatStream, &sizeX, 1);
        cxx::read_elements(gCacheDir->mDatStream, &sizeY, 1);
        cxx::read_elements(gCacheDir->mDatStream, &dataSize, 1);  // need correction

        unsigned short shortDataX = 0;
        unsigned short shortDataY = 0;
        cxx::read_elements(gCacheDir->mDatStream, &shortDataX, 1);
        cxx::read_elements(gCacheDir->mDatStream, &shortDataY, 1);

        unsigned int someFlags = 0;
        cxx::read_elements(gCacheDir->mDatStream, &someFlags, 1);

        gCacheDir->mEntries[i].mSizeX = sizeX;
        gCacheDir->mEntries[i].mSizeY = sizeY;
        gCacheDir->mEntries[i].mDataLength = dataSize - 8;
        gCacheDir->mEntries[i].mSizeX2 = shortDataX;
        gCacheDir->mEntries[i].mSizeY2 = shortDataY;
        gCacheDir->mEntries[i].mHasAlpha = (someFlags >> 7) > 0;
        gCacheDir->mEntries[i].mDataStartLocation = offset + 20;
    }

    return true;
}

static bool DK2_GetEngineTextureDesc(int theIndex, DK2EngineTextureEntry* theOutput)
{
    int numEntries = gCacheDir->mEntries.size();
    if (gCacheDir && theIndex < numEntries)
    {
        theOutput[0] = gCacheDir->mEntries[theIndex];
        return true;
    }
    return false;
}

static bool DK2_ExtractEngineTexturePixels(int theIndex, unsigned char* theOutputBuffer)
{
    int numEntries = gCacheDir->mEntries.size();
    if (gCacheDir && theIndex < numEntries)
    {
        stream_uint32 uint32stream (gCacheDir->mDatStream, 
            gCacheDir->mEntries[theIndex].mDataStartLocation, 
            gCacheDir->mEntries[theIndex].mDataLength);

        dd_texture(&uint32stream, theOutputBuffer, 
            gCacheDir->mEntries[theIndex].mSizeX * 4,
            gCacheDir->mEntries[theIndex].mSizeX,
            gCacheDir->mEntries[theIndex].mSizeY, gCacheDir->mEntries[theIndex].mHasAlpha);
        return true;
    }
    return false;
}

static bool DK2_OpenEngineTexturesCache(const char* theCacheLocation)
{
    cxx_assert(gCacheDir == nullptr);
    if (gCacheDir)
    {
        return true;
    }

    std::string pathDat = cxx::va("%s/EngineTextures.dat", theCacheLocation);
    std::string pathDir = cxx::va("%s/EngineTextures.dir", theCacheLocation);

    gCacheDir = new DK2EngineTexturesCacheDir;

    // read dir content
    if (!dk2_read_dir_content(pathDir.c_str(), pathDat.c_str()))
    {
        SafeDelete(gCacheDir);
        return false;
    }
    return true;
}

static void DK2_CloseEngineTexturesCache()
{
    SafeDelete(gCacheDir);
}

static bool DK2_GetEngineTexturesCount(int* theOutput)
{
    if (!gCacheDir)
    {
        return false;
    }
    *theOutput = gCacheDir->mEntries.size();
    return true;
}

//////////////////////////////////////////////////////////////////////////

inline int GetMipmapLevelFromName(const std::string& theString)
{
    const int stringLength = theString.length();
    if (stringLength < 3)
    {
        return -1;
    }
    bool isMipmap = (theString[stringLength - 3] == 'M') && (theString[stringLength - 2] == 'M');
    if (!isMipmap)
    {
        return -1;
    }

    char mchar = theString[stringLength - 1];
    if (mchar < '0' || mchar > '9')
    {
        return -1;
    }
    return mchar - '0';
}

//////////////////////////////////////////////////////////////////////////

DK2EngineTexturesCache::DK2EngineTexturesCache()
{}

DK2EngineTexturesCache::~DK2EngineTexturesCache()
{
    Shutdown();
}

bool DK2EngineTexturesCache::ScanDungeonKeeperTexturesCache(const std::string& theTexturesCachePath)
{
    DK2_CloseEngineTexturesCache();
    if (!DK2_OpenEngineTexturesCache(theTexturesCachePath.c_str()))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot open engine textures cache");
        return false;
    }
    int numEntries;
    if (!DK2_GetEngineTexturesCount(&numEntries))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot query engine textures cache entries");
        return false;
    }
    gConsole.LogMessage(eLogLevel_Info, "Found %d entries in engine textures cache", numEntries);

    mEntries.reserve(2048);
    for (int ientry = 0; ientry < numEntries; ++ientry)
    {
        DK2EngineTextureEntry engineTextureDesc;
        if (!DK2_GetEngineTextureDesc(ientry, &engineTextureDesc))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot query information of %d entry", ientry);
            continue;
        }
        std::string textureName = engineTextureDesc.mName;

        const int iMip = GetMipmapLevelFromName(textureName);
        if ((iMip < 0) || (iMip > MAX_ENGINE_TEXTURE_MIPS - 1))
        {
            cxx_assert(false);

            gConsole.LogMessage(eLogLevel_Warning, "Ignore texture '%s'", engineTextureDesc.mName);
            continue;
        }
        // remove MMn suffix
        textureName.erase(
            textureName.begin() + textureName.length() - 3,
            textureName.end());

        DK2EngineTextureID textureID;

        auto entry_iterator = mIndices.find(textureName);
        if (entry_iterator == mIndices.end())
        {
            textureID = mEntries.size();
            mIndices.emplace(textureName, textureID);
            mEntries.push_back({});
        }
        else
        {
            textureID = entry_iterator->second;
        }

        DK2EngineTextureDesc& textureEntry = mEntries[textureID];
        textureEntry.mHasAlpha = textureEntry.mHasAlpha || engineTextureDesc.mHasAlpha;
        textureEntry.mMips[iMip].mDimX = engineTextureDesc.mSizeX;
        textureEntry.mMips[iMip].mDimY = engineTextureDesc.mSizeY;
        textureEntry.mMips[iMip].mEntryIndex = ientry;
        ++textureEntry.mMipsCount;
    }
    gConsole.LogMessage(eLogLevel_Info, "Found %d unique entries in textures cache", mEntries.size());
    return true;
}

void DK2EngineTexturesCache::Shutdown()
{
    DK2_CloseEngineTexturesCache();
    mIndices.clear();
    mEntries.clear();
}

bool DK2EngineTexturesCache::ExtractTexture(DK2EngineTextureID theTextureID, BitmapImage& outputBitmap) const
{
    bool isTextureID = theTextureID < mEntries.size();

    cxx_assert(isTextureID);
    if (!isTextureID)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Invalid engine texture id %d", theTextureID);
        return false;
    }

    const DK2EngineTextureDesc& texEntry = mEntries[theTextureID];
    
    if (texEntry.mMips[0].mEntryIndex < 0)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Engine texture mip #0 is not defined for %d", theTextureID);
        return false;
    }

    // create image
    Point2D dims {texEntry.mMips[0].mDimX, texEntry.mMips[0].mDimY};
    outputBitmap.Clear();
    if (!outputBitmap.Create(ePixelFormat_RGBA8, dims, nullptr))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot create memory bitmap for %d", theTextureID);
        return false;
    }

    unsigned char* pixels = outputBitmap.GetMipPixels(0);
    if (!DK2_ExtractEngineTexturePixels(texEntry.mMips[0].mEntryIndex, pixels))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot extract texture pixels for %d", theTextureID);
        return false;
    }

    // extract mipmaps
    for (int imip = 1; imip < MAX_ENGINE_TEXTURE_MIPS; ++imip)
    {
        const int texindex = texEntry.mMips[imip].mEntryIndex;
        if (texindex < 0)
        {
            continue;
        }

        DK2EngineTextureEntry desc;
        if (!DK2_GetEngineTextureDesc(texindex, &desc))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot query texture desc mipmap %d for %d", imip, theTextureID);
            continue;
        }
        Point2D mipDims {desc.mSizeX, desc.mSizeY};
        outputBitmap.AddMipLevel(mipDims, &pixels);
        if (!DK2_ExtractEngineTexturePixels(texindex, pixels))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot extract texture mipmap %d for %d", imip, theTextureID);
            continue;
        }
    }

    outputBitmap.SetHasAlphaHint(texEntry.mHasAlpha);
    return true;
}


bool DK2EngineTexturesCache::Decompress444(const ByteArray& textureData, BitmapImage& outputImage)
{
    outputImage.Clear();

    unsigned short sizex;
    unsigned short sizey;
    unsigned int flags;

    // read metadata
    {
        cxx::memory_istream memorystream((char*)textureData.data(), (char*)textureData.data() + textureData.size());
        std::istream instream(&memorystream);
        if (!instream.read((char*)&sizex, sizeof(sizex)) || 
            !instream.read((char*)&sizey, sizeof(sizey)) || 
            !instream.read((char*)&flags, sizeof(flags)))
        {
            cxx_assert(false);
            return false;
        }
    }

    bool hasAlpha = (flags & 0x80) > 0;

    unsigned int metadataLength = sizeof(sizex) + sizeof(sizey) + sizeof(flags);
    unsigned int dataLength = textureData.size() - metadataLength;

    if (!outputImage.Create(ePixelFormat_RGBA8, {sizex, sizey}, COLOR_BLACK))
    {
        cxx_assert(false);
        return false;
    }

    cxx::memory_istream memorystream((char*)&textureData[0], (char*) &textureData[0] + textureData.size());
    std::istream instream(&memorystream);
    stream_uint32 uint32stream (instream, metadataLength, dataLength);

    dd_texture_444(&uint32stream, outputImage.GetMipPixels(0), sizex * 4, sizex, sizey, hasAlpha);

    outputImage.SetHasAlphaHint(hasAlpha);
    return true;
}

bool DK2EngineTexturesCache::FindTextureByName(const std::string& theTextureName, DK2EngineTextureID& theTextureID) const
{
    auto found_iterator = mIndices.find(theTextureName);
    if (found_iterator != mIndices.end())
    {
        theTextureID = found_iterator->second;
        return true;
    }
    return false;
}

bool DK2EngineTexturesCache::ExtractTextureInfo(DK2EngineTextureID theTextuteID, DK2EngineTextureDesc& outputInfo) const
{
    cxx_assert(theTextuteID < mEntries.size());
    if (theTextuteID < mEntries.size())
    {
        outputInfo = mEntries[theTextuteID];
        return true;
    }
    gConsole.LogMessage(eLogLevel_Warning, "Invalid engine texture id %d", theTextuteID);
    return false;
}

bool DK2EngineTexturesCache::GetTextureNameByID(DK2EngineTextureID theTextureID, std::string& textureName) const
{
    textureName.clear();
    for (const auto& roller: mIndices)
    {
        if (roller.second == theTextureID)
        {
            textureName = roller.first;
            return true;
        }
    }
    return false;
}

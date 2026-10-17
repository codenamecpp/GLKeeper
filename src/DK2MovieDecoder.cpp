#include "stdafx.h"
#include "DK2MovieDecoder.h"

//////////////////////////////////////////////////////////////////////////

// 'pIQT' as little-endian fourcc
#define FOURCC_PIQT 0x54514970u

//////////////////////////////////////////////////////////////////////////

constexpr uint32_t Fourcc(char a, char b, char c, char d)
{
    return (uint32_t)(uint8_t)a | ((uint32_t)(uint8_t)b << 8) | ((uint32_t)(uint8_t)c << 16) | ((uint32_t)(uint8_t)d << 24);
}

//////////////////////////////////////////////////////////////////////////

inline uint32_t rl32(const uint8_t* p) 
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
inline int16_t rl16s(const uint8_t* p) 
{ 
    return (int16_t)(uint16_t)(p[0] | (p[1] << 8)); 
}
inline int align16(int v) { return (v + 15) & ~15; }
inline int16_t clip16(int v) 
{ 
    return (int16_t)(v < -32768 ? -32768 : v > 32767 ? 32767 : v); 
}

//////////////////////////////////////////////////////////////////////////

// AAN scales
static const int32_t TQI_BASE1[64] = 
{
    8192,5906,6270,6967,8192,10426,15137,29692,5906,4258,4520,5023,5906,7517,10913,21407,6270,4520,4799,5332,
    6270,7980,11585,22725,6967,5023,5332,5925,6967,8867,12873,25251,8192,5906,6270,6967,8192,10426,15137,29692,
    10426,7517,7980,8867,10426,13270,19266,37791,15137,10913,11585,12873,15137,19266,27969,54864,29692,21407,22725,
    25251,29692,37791,54864,107619
};

// MPEG-1 default intra matrix
static const int32_t TQI_BASE2[64] = 
{
    8,16,19,22,26,27,29,34,16,16,22,24,27,29,34,37,19,22,26,27,29,34,34,38,22,22,26,27,29,34,37,40,22,26,27,29,32,35,40,
    48,26,27,29,32,35,40,48,58,26,27,29,34,38,46,56,69,27,29,35,38,46,56,69,83
}; 

// scan index k (0..63) -> position pos = row*8+col in the 8x8 coefficient array
static const uint8_t TQI_SCAN[64] = 
{
    0,8,1,2,9,16,24,17,10,3,4,11,18,25,32,40,33,26,19,12,5,6,13,20,27,34,41,48,56,49,42,35,28,21,14,7,15,22,29,36,43,50,
    57,58,51,44,37,30,23,31,38,45,52,59,60,53,46,39,47,54,61,62,55,63
};

// DC VLC lookup: byte = (dc_size<<4)|code_len ; luma indexed by next 7 bits, chroma by next 8 bits
static const uint8_t TQI_DC_LUMA[128] = 
{
    18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,34,34,34,34,34,34,34,
    34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,51,51,51,51,
    51,51,51,51,51,51,51,51,51,51,51,51,67,67,67,67,67,67,67,67,67,67,67,67,67,67,67,67,84,84,84,84,84,84,84,84,101,101,101,
    101,118,118,135,0
};

static const uint8_t TQI_DC_CHROMA[256] = 
{
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,
    18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,
    34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,34,
    34,34,34,34,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,68,68,68,68,68,68,
    68,68,68,68,68,68,68,68,68,68,85,85,85,85,85,85,85,85,102,102,102,102,119,119,136,0
};

// AC VLC (MPEG-1 table B.14)
struct TqiAc
{
    const char* code;
    uint8_t run1;
    uint8_t level;
};

static const TqiAc TQI_AC[]=
{
    {"10",65,0},
    {"11",1,1},
    {"011",2,1},
    {"0100",1,2},
    {"0101",3,1},
    {"00101",1,3},
    {"00110",5,1},
    {"00111",4,1},
    {"000001",66,0},
    {"000100",8,1},
    {"000101",7,1},
    {"000110",2,2},
    {"000111",6,1},
    {"0000100",3,2},
    {"0000101",10,1},
    {"0000110",1,4},
    {"0000111",9,1},
    {"00100000",14,1},
    {"00100001",1,6},
    {"00100010",13,1},
    {"00100011",12,1},
    {"00100100",4,2},
    {"00100101",2,3},
    {"00100110",1,5},
    {"00100111",11,1},
    {"0000001000",17,1},
    {"0000001001",6,2},
    {"0000001010",1,7},
    {"0000001011",3,3},
    {"0000001100",2,4},
    {"0000001101",16,1},
    {"0000001110",15,1},
    {"0000001111",5,2},
    {"000000010000",1,11},
    {"000000010001",9,2},
    {"000000010010",5,3},
    {"000000010011",1,10},
    {"000000010100",3,4},
    {"000000010101",8,2},
    {"000000010110",22,1},
    {"000000010111",21,1},
    {"000000011000",1,9},
    {"000000011001",20,1},
    {"000000011010",19,1},
    {"000000011011",2,5},
    {"000000011100",4,3},
    {"000000011101",1,8},
    {"000000011110",7,2},
    {"000000011111",18,1},
    {"0000000010000",11,2},
    {"0000000010001",10,2},
    {"0000000010010",6,3},
    {"0000000010011",4,4},
    {"0000000010100",3,5},
    {"0000000010101",2,7},
    {"0000000010110",2,6},
    {"0000000010111",1,15},
    {"0000000011000",1,14},
    {"0000000011001",1,13},
    {"0000000011010",1,12},
    {"0000000011011",27,1},
    {"0000000011100",26,1},
    {"0000000011101",25,1},
    {"0000000011110",24,1},
    {"0000000011111",23,1},
    {"00000000010000",1,31},
    {"00000000010001",1,30},
    {"00000000010010",1,29},
    {"00000000010011",1,28},
    {"00000000010100",1,27},
    {"00000000010101",1,26},
    {"00000000010110",1,25},
    {"00000000010111",1,24},
    {"00000000011000",1,23},
    {"00000000011001",1,22},
    {"00000000011010",1,21},
    {"00000000011011",1,20},
    {"00000000011100",1,19},
    {"00000000011101",1,18},
    {"00000000011110",1,17},
    {"00000000011111",1,16},
    {"000000000010000",1,40},
    {"000000000010001",1,39},
    {"000000000010010",1,38},
    {"000000000010011",1,37},
    {"000000000010100",1,36},
    {"000000000010101",1,35},
    {"000000000010110",1,34},
    {"000000000010111",1,33},
    {"000000000011000",1,32},
    {"000000000011001",2,14},
    {"000000000011010",2,13},
    {"000000000011011",2,12},
    {"000000000011100",2,11},
    {"000000000011101",2,10},
    {"000000000011110",2,9},
    {"000000000011111",2,8},
    {"0000000000010000",2,18},
    {"0000000000010001",2,17},
    {"0000000000010010",2,16},
    {"0000000000010011",2,15},
    {"0000000000010100",7,3},
    {"0000000000010101",17,2},
    {"0000000000010110",16,2},
    {"0000000000010111",15,2},
    {"0000000000011000",14,2},
    {"0000000000011001",13,2},
    {"0000000000011010",12,2},
    {"0000000000011011",32,1},
    {"0000000000011100",31,1},
    {"0000000000011101",30,1},
    {"0000000000011110",29,1},
    {"0000000000011111",28,1},
    {0,0,0}
};

//////////////////////////////////////////////////////////////////////////

namespace tqi 
{
 
    struct FrameInfo 
    {
        int width {};
        int height {}; 
        int quant {};
        int flags {}; 
    };
 
    namespace detail 
    {
 
        constexpr int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
 
        // precomputed YUV -> RGB colour tables
        // fixed-point, 9-bit fields
        struct ColorTables 
        {
            // packed contributions:
            //   a[] = Y contribution  (R/G/B share the same Y term)
            //   b[] = U contribution  (mainly G and B)
            //   c[] = V contribution  (mainly R and G)
            uint32_t a[512] {}, b[512] {}, c[512] {};

            // final 8-bit channel lookup after the packed sum is formed
            uint8_t  r[512] {}, g[512] {}, bl[512] {};

            // ITU-R BT.601 coefficients
            static constexpr double kCbToG =  0.3441;   // -0.344136 * Cb  -> G
            static constexpr double kCbToB = -1.772;    //  1.772    * Cb  -> B
            static constexpr double kCrToR = -1.402;    //  1.402    * Cr  -> R
            static constexpr double kCrToG =  0.7141;   // -0.714136 * Cr  -> G

            // bias added to the Y contribution
            static constexpr uint32_t kYBias = 0x4883d8a9u;

            constexpr ColorTables() 
            {
                // build the three packed contribution tables for every possible
                // signed sample in [-256 ... 255].  The sample is first clamped
                // to the range the original engine used (-64 ... 63)
                for (int sample = -256; sample < 256; ++sample) 
                {
                    const int clamped = clampi(sample, -64, 63);
                    const uint32_t idx = static_cast<uint32_t>(sample - 192) & 0x1FFu;

                    // Y contribution
                    uint32_t yTerm = static_cast<uint32_t>(clamped);
                    yTerm = (yTerm << 12) + yTerm;          // * 4097
                    yTerm = (yTerm << 11) + static_cast<uint32_t>(clamped);
                    a[idx] = yTerm + kYBias;

                    // U / V contributions (floating-point, truncated toward zero)
                    const double centered = static_cast<double>(clamped) + 0.5;

                    const uint32_t uG = static_cast<uint32_t>(0.5 - centered * kCbToG);
                    const uint32_t uB = static_cast<uint32_t>(0.5 - centered * kCbToB);
                    const uint32_t vR = static_cast<uint32_t>(0.5 - centered * kCrToR);
                    const uint32_t vG = static_cast<uint32_t>(0.5 - centered * kCrToG);

                    b[idx] = ((uG & 0x1FFu) << 11) | (uB & 0x1FFu);
                    c[idx] = (((vG & 0x1FFu) + ((vR & 0x1FFu) << 12)) << 11);
                }

                // final 8-bit expansion tables, linear mapping with offset
                for (int i = 0; i < 512; ++i) 
                {
                    r[i]  = static_cast<uint8_t>(clampi(-177 + 2 * i, 0, 255));
                    g[i]  = static_cast<uint8_t>(clampi(-133 + 2 * i, 0, 255));
                    bl[i] = static_cast<uint8_t>(clampi(-225 + 2 * i, 0, 255));
                }
            }
        };
        static const ColorTables kColor {};
 
        // AC VLC: 16-bit window -> uint16 { len:5 | run:5 | level:6 }.  level==0 && run==0 -> EOB, level==0 && run==1 -> ESC
        struct AcTable 
        {
        public:
            AcTable() 
            {
                std::memset(e, 0, sizeof(e));
                for (const TqiAc* c = TQI_AC; c->code; c++) 
                {
                    const int len = static_cast<int>(std::strlen(c->code)); 
                    uint32_t v = 0;
                    for (int i = 0; i < len; i++) 
                    {
                        v = (v << 1) | (c->code[i] == '1');
                    }
                    uint16_t ent;
                    if (c->run1 == 65)      ent = static_cast<uint16_t>(len << 11);                 // EOB
                    else if (c->run1 == 66) ent = static_cast<uint16_t>((len << 11) | (1 << 6));    // ESC
                    else                    ent = static_cast<uint16_t>((len << 11) | ((c->run1 - 1) << 6) | c->level);

                    const uint32_t lo = v << (16 - len); 
                    const uint32_t hi = lo + (1u << (16 - len));
                    for (uint32_t w = lo; w < hi; w++) 
                    {
                        e[w] = ent;
                    }
                } // for
            }
        public:
            uint16_t e[1 << 16];
        };

        inline const AcTable& acTable() { static const AcTable t; return t; }
 
        // bitstream reader for TQI: little-endian 32-bit words, bits taken MSB-first
        struct Bits 
        {
            const uint8_t* data;      // raw payload
            uint32_t       size;      // size in bytes
            uint64_t       bitPos;    // current bit offset

            // read a 32-bit little-endian word at the given word index
            inline uint32_t readWord(uint32_t wordIndex) const 
            {
                const uint32_t byteOffset = wordIndex * 4; 
                uint32_t value = 0;

                if (byteOffset + 4 <= size) 
                { 
                    std::memcpy(&value, data + byteOffset, 4); 
                    return toHostEndian(value); 
                }

                // partial word at the end of the buffer
                for (uint32_t i = 0; i < 4 && (byteOffset + i) < size; ++i) 
                {
                    value |= static_cast<uint32_t>(data[byteOffset + i]) << (8 * i);
                }
                return value;
            }

            static uint32_t toHostEndian(uint32_t v) 
            {
        #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                return __builtin_bswap32(v);
        #else
                return v;
        #endif
            }

            // peek the next 32 bits without advancing the position
            inline uint32_t peek32() const 
            {
                const uint32_t wordIdx  = static_cast<uint32_t>(bitPos >> 5); 
                const uint32_t shift    = static_cast<uint32_t>(bitPos & 31);
                const uint32_t lowWord  = readWord(wordIdx);

                if (shift == 0) 
                    return lowWord;

                return (lowWord << shift) | (readWord(wordIdx + 1) >> (32 - shift));
            }

            inline bool overrun() const 
            { 
                return bitPos > static_cast<uint64_t>(size) * 8; 
            }
        };
 
        // AAN / floating-point IDCT constants
        constexpr double kAanA = 0.5411961078643799;   // cos(3*pi/8) / (sqrt(2) * cos(pi/8) )
        constexpr double kAanB = 1.3065630197525024;
        constexpr double kAanC = 0.3826834261417389;   // cos(3*pi/8)
        constexpr int64_t kSqrt2Scaled = 0x5a82799a;   // ~ sqrt(2) * 2^30  (fixed-point)

        // round-to-nearest-even via the classic "add 1.5 * 2^52 and read low dword" trick 
        inline int32_t roundNearestEven(double x) 
        {          
            double tmp = x + 6755399441055744.0;   // 1.5 * 2^52
            uint64_t bits; 
            std::memcpy(&bits, &tmp, sizeof(bits)); 
            return static_cast<int32_t>(static_cast<uint32_t>(bits));
        }

        // 1-D 8-point inverse DCT (AAN algorithm, fixed-point + float hybrid)
        inline void idct8(const int32_t* in, int32_t* out) 
        {
            // odd part
            const int32_t t35 = in[3] + in[5];
            const int32_t A   = in[5] - in[3];
            const int32_t t17 = in[7] + in[1];
            const int32_t B   = in[1] - in[7];
            const int32_t S   = t35 + t17;
            const int32_t D   = t17 - t35;

            const double Z  = static_cast<double>(A + B) * kAanC;
            const int32_t Y1 = roundNearestEven(static_cast<double>(A) * kAanA + Z);
            const int32_t Y2 = roundNearestEven(static_cast<double>(B) * kAanB - Z);

            // (D * sqrt(2)) in fixed-point, then *2
            const int32_t E = static_cast<int32_t>(
                static_cast<uint32_t>(static_cast<int32_t>((D * kSqrt2Scaled) >> 32)) << 1);

            const int32_t r1 = Y1 + E;
            const int32_t r2 = E  + Y2;
            const int32_t r3 = Y2 + S;

            // Even part
            const int32_t d26 = in[2] - in[6];
            const int32_t s04 = in[4] + in[0];
            const int32_t d04 = in[0] - in[4];

            const int32_t F = static_cast<int32_t>(
                static_cast<uint32_t>(static_cast<int32_t>((d26 * kSqrt2Scaled) >> 32)) << 1);

            const int32_t s26 = in[6] + in[2] + F;
            const int32_t e0  = s26 + s04;
            const int32_t e3  = s04 - s26;
            const int32_t e1  = F   + d04;
            const int32_t e2  = d04 - F;

            out[0] = e0 + r3;  out[7] = e0 - r3;
            out[1] = e1 + r2;  out[6] = e1 - r2;
            out[2] = e2 + r1;  out[5] = e2 - r1;
            out[3] = e3 + Y1;  out[4] = e3 - Y1;
        }

        // convert an IDCT sample to an index into the colour tables
        inline uint16_t sampleToColorIndex(int32_t sample) 
        { 
            return static_cast<uint16_t>(clampi(sample, -256, 255) + 256); 
        }
 
        // full 8*8 inverse DCT + colour-index conversion
        // uses the classic row-column approach with a DC-only fast path
        inline void idctBlock(const int32_t* coef, uint16_t* out, int stride) 
        {
            int32_t tmp[8][8];
            int32_t col[8];

            // Rows
            for (int row = 0; row < 8; ++row) 
            {
                const int32_t* src = coef + row * 8;

                // fast path: only DC coefficient is non-zero
                if ((src[1] | src[2] | src[3] | src[4] | src[5] | src[6] | src[7]) == 0) 
                { 
                    for (int j = 0; j < 8; ++j) 
                    {
                        col[j] = src[0];
                    }
                }
                else 
                {
                    idct8(src, col);
                }

                // transpose into temporary matrix
                for (int j = 0; j < 8; ++j) 
                {
                    tmp[j][row] = col[j];
                }
            }

            // columns
            for (int colIdx = 0; colIdx < 8; ++colIdx) 
            {
                idct8(tmp[colIdx], col);
                for (int i = 0; i < 8; ++i) 
                {
                    out[colIdx * stride + i] = sampleToColorIndex(col[i] >> 17);
                }
            }
        }
 
        // read a differential DC coefficient (MPEG-1 style VLC)
        // returns the signed delta that must be added to the predictor
        inline int readDc(Bits& bits, bool isChroma) 
        {
            const uint32_t window = bits.peek32();
            const uint8_t entry = isChroma 
                ? TQI_DC_CHROMA[window >> 24] 
                : TQI_DC_LUMA[window >> 25];

            const int size = entry >> 4;          // number of magnitude bits
            bits.bitPos += entry & 0x0F;          // consume the VLC code itself

            if (size == 0) 
                return 0;

            uint32_t magnitude = bits.peek32() >> (32 - size);
            bits.bitPos += size;

            // sign-extend if the high bit is clear (two's-complement style)
            if (((magnitude >> (size - 1)) & 1) == 0) 
            {
                magnitude -= (1u << size) - 1;
            }

            return static_cast<int32_t>(magnitude);
        }
 
        // decode one 8x8 block (DC + AC coefficients -> colour indices).
        inline void decodeBlock(Bits& bits, const int32_t* quant, int& dcPredictor, bool isChroma, uint16_t* out, int stride) 
        {
            int32_t coef[64] {};

            // DC
            dcPredictor += readDc(bits, isChroma);
            coef[0] = static_cast<int32_t>(
                static_cast<uint32_t>(dcPredictor) * static_cast<uint32_t>(quant[0]));

            // AC
            const uint16_t* acLut = acTable().e;
            int scanPos = 0;
            bool hasAc = false;

            for (;;) 
            {
                const uint32_t window = bits.peek32() >> 16;
                const uint16_t entry  = acLut[window];

                if (entry == 0)                 // invalid / end of stream
                    break;

                const int codeLen = entry >> 11;
                const int run     = (entry >> 6) & 31;
                int level         = entry & 63;
                bits.bitPos += codeLen;

                if (level == 0) 
                {
                    if (run == 0)               // EOB
                        break;

                    // esc code: 6-bit run + 8-bit level (possibly extended)
                    const uint32_t esc = bits.peek32() >> 18;
                    bits.bitPos += 14;

                    const int levelByte = esc & 0xFF;
                    scanPos += static_cast<int>((esc >> 8) & 63) + 1;

                    if (levelByte == 0) 
                    {
                        level = static_cast<int>(bits.peek32() >> 24);
                        bits.bitPos += 8;
                    }
                    else if (levelByte == 0x80) 
                    {
                        level = static_cast<int>(bits.peek32() >> 24) - 256;
                        bits.bitPos += 8;
                    }
                    else 
                    {
                        level = (levelByte > 0x80) ? levelByte - 256 : levelByte;
                    }
                } 
                else 
                {
                    scanPos += run + 1;

                    // sign bit follows the VLC code
                    if (bits.peek32() >> 31) 
                    {
                        level = -level;
                    }
                    bits.bitPos += 1;
                }

                if (scanPos > 63) 
                    break;

                const int zigzag = TQI_SCAN[scanPos];
                coef[zigzag] = static_cast<int32_t>(
                    static_cast<uint32_t>(level) * static_cast<uint32_t>(quant[zigzag]));
                hasAc = true;
            }

            // DC-only fast path (very common for flat areas)
            if (!hasAc) 
            {    
                const uint16_t value = sampleToColorIndex(coef[0] >> 17);
                for (int y = 0; y < 8; ++y) 
                {
                    for (int x = 0; x < 8; ++x) 
                    {
                        out[y * stride + x] = value;
                    }
                }
            } 
            else 
            {
                idctBlock(coef, out, stride);
            }
        }
 
        // write a single RGB24 pixel
        inline void putRgb(uint8_t* dst, unsigned r, unsigned g, unsigned b) 
        {
            dst[0] = static_cast<uint8_t>(r); 
            dst[1] = static_cast<uint8_t>(g); 
            dst[2] = static_cast<uint8_t>(b);
        }
 
    }  // namespace detail
    
    //////////////////////////////////////////////////////////////////////////
 
    class Decoder 
    {
    public:
        // parse the 8-byte frame header (payload = chunk data after the 8-byte chunk header)
        static bool readHeader(const uint8_t* p, uint32_t n, FrameInfo& fi) 
        {
            if (n < 8) 
                return false;
            fi.width = p[0] | (p[1] << 8); 
            fi.height = p[2] | (p[3] << 8); 
            fi.quant = p[4]; 
            fi.flags = p[7];
            return fi.width > 0 && fi.height > 0;
        }
 
        // decode one frame into dst
        bool decode(const uint8_t* payload, uint32_t size, uint8_t* dst, std::ptrdiff_t stride) 
        {
            return run(payload, size, dst, stride);
        }
 
    private:
        int quant_ = -1;
        int32_t q_[64] {};
 
    private:
        void makeQ(int quant) 
        {
            if (quant == quant_) 
                return;

            for (int i = 0; i < 64; i++) 
            {
                if (quant == 100) 
                {
                    q_[i] = TQI_BASE1[i] * 8;
                }
                else 
                {
                    const int64_t qi = static_cast<int64_t>(TQI_BASE2[i]) * (107 - quant) * 4096;
                    q_[i] = static_cast<int32_t>((static_cast<int64_t>(TQI_BASE1[i]) * qi + 0x8000) >> 16);
                }
            }
            q_[0] = TQI_BASE1[0] * 8;
            quant_ = quant;
        }
 
        bool run(const uint8_t* p, uint32_t n, uint8_t* dst, std::ptrdiff_t stride) 
        {
            static const int bytesPerPixel = 3;

            FrameInfo fi;
            if (!readHeader(p, n, fi)) 
                return false;

            makeQ(fi.quant);
            const auto& color = detail::kColor;
            detail::Bits bits{p + 8, n - 8, 0};

            int dcPred[3] = {0, 0, 0};          // Y, Cb, Cr predictors
            const int mbWidth  = (fi.width  + 15) / 16;
            const int mbHeight = (fi.height + 15) / 16;

            uint16_t Y[16 * 16], U[8 * 8], V[8 * 8];

            for (int mbY = 0; mbY < mbHeight; ++mbY) 
            {
                for (int mbX = 0; mbX < mbWidth; ++mbX) 
                {
                    // four 8x8 luma blocks + two 8x8 chroma blocks (4:2:0)
                    detail::decodeBlock(bits, q_, dcPred[0], false, Y,              16);
                    detail::decodeBlock(bits, q_, dcPred[0], false, Y + 8,          16);
                    detail::decodeBlock(bits, q_, dcPred[0], false, Y + 8 * 16,     16);
                    detail::decodeBlock(bits, q_, dcPred[0], false, Y + 8 * 16 + 8, 16);
                    detail::decodeBlock(bits, q_, dcPred[1], true,  U, 8);
                    detail::decodeBlock(bits, q_, dcPred[2], true,  V, 8);

                    const int x0 = mbX * 16;
                    const int y0 = mbY * 16;
                    const int blockW = (fi.width  - x0 < 16) ? fi.width  - x0 : 16;
                    const int blockH = (fi.height - y0 < 16) ? fi.height - y0 : 16;

                    for (int y = 0; y < blockH; ++y) 
                    {
                        uint8_t* dstPixel = dst 
                            + static_cast<std::ptrdiff_t>(y0 + y) * stride 
                            + static_cast<std::ptrdiff_t>(x0) * bytesPerPixel;

                        const uint16_t* rowY = Y + y * 16; 
                        const uint16_t* rowU = U + (y >> 1) * 8; 
                        const uint16_t* rowV = V + (y >> 1) * 8;

                        for (int x = 0; x < blockW; ++x, dstPixel += bytesPerPixel) 
                        {
                            // combine the three precomputed contributions and expand to 8-bit RGB
                            const uint32_t packed = color.a[rowY[x]] 
                                                  + color.b[rowU[x >> 1]] 
                                                  + color.c[rowV[x >> 1]];
                            detail::putRgb(dstPixel,
                                color.r[packed >> 23],
                                color.g[(packed & 0xFF800u) >> 11],
                                color.bl[packed & 0x1FFu]);
                        }
                    }
                }
            }
            return !bits.overrun();
        }
    };
 
}  // namespace tqi

//////////////////////////////////////////////////////////////////////////

// audio decoding

// a movie may contain several audio tracks (e.g. languages): 
// N header chunks at the start of the file and, after every video frame, N consecutive data chunks - the n-th one belongs to track n

// codecs:
//   0  PCM s16le:   u32 frames; then frames*channels samples (interleaved)
//   7  EA ADPCM:    u32 (ignored), s16 curL, prevL, curR, prevR (mono: only the first two are used), then blocks
//                   stereo: 30 bytes = [coefL<<4|coefR][shiftL<<4|shiftR][28 bytes: hiNibble=L loNibble=R]
//                   mono:   15 bytes = [coef<<4|shift][14 bytes, hi nibble first]      -> 28 frames per block
//   10 IMA SEAD:    stereo IMA ADPCM, one byte per frame (hi nibble L, lo nibble R), state kept between chunks

namespace dk2audio 
{

    struct Format
    {
        bool valid = false;
        int bits = 16;
        int channels = 1;
        int sampleRate = 22050;
        int codec = 0;
    };

    enum eCodec { eCodec_Pcm16 = 0, eCodec_EaAdpcm = 7, eCodec_ImaSead = 10 };

    inline bool IsHeaderTag(uint32_t t) { return (t == Fourcc('S','C','H','l')) || (t == Fourcc('1','S','N','h')) || (t == Fourcc('S','E','A','D')); }
    inline bool IsDataTag(uint32_t t)   { return (t == Fourcc('S','C','D','l')) || (t == Fourcc('1','S','N','d')) || (t == Fourcc('S','N','D','C')); }
    inline bool IsOtherAudioTag(uint32_t t)
    {
        return (t == Fourcc('S','C','E','l')) || 
            (t == Fourcc('1','S','N','e')) || 
            (t == Fourcc('S','E','N','D')) ||
            (t == Fourcc('S','C','C','l')) || 
            (t == Fourcc('S','C','L','l'));
    }

    namespace detail 
    {

        // {tag, len, len bytes BE}, sign-extended for len 1..3
        inline bool ReadElement(const uint8_t*& p, const uint8_t* end, int32_t& value)
        {
            if (end - p < 2) 
                return false;

            const int n = p[1];
            if (end - p < 2 + n) 
                return false;

            uint32_t v = 0;
            for (int i = 0; i < n; i++) 
            {
                v = (v << 8) | p[2 + i];
            }
            if (n == 1 && v > 0x7f) 
            {
                v -= 0x100;
            }
            else if (n == 2 && v > 0x7fff) 
            {
                v -= 0x10000;
            }
            else if (n == 3 && v > 0x7fffff) 
            {
                v -= 0x1000000;
            }
            value = (int32_t)v;
            p += 2 + n;
            return true;
        }

        // ea_adpcm_table: coef1[i] = T[i], coef2[i] = T[i + 4]
        static const int EA_TABLE[20] = { 0, 240, 460, 392, 0, 0, -208, -220, 0, 1, 3, 4, 7, 8, 10, 11, 0, -1, -3, -4 };

        // standard IMA tables
        static const int IMA_STEP[89] = 
        {
            7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,
            230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,
            2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,
            20350,22385,24623,27086,29794,32767 
        };
        static const int IMA_INDEX_DELTA[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

    } // namespace detail

    inline bool ParseHeaderChunk(const uint8_t* p, uint32_t size, Format& f)
    {
        f = Format();
        const uint8_t* end = p + size;
        int32_t v = 0;
        if (size <= 4) 
            return false;

        // skip leading elements until the 0xFD sub-header
        p += 4;
        while ((p < end) && (*p != 0xFD))
        {
            if (!detail::ReadElement(p, end, v)) 
                return false;
        }

        if (p >= end) 
            return false;
        p++; // skip 0xFD
        while ((p < end) && (*p != 0x8A))
        {
            const uint8_t tag = *p;
            if (!detail::ReadElement(p, end, v)) 
                return false;
            switch (tag)
            {
                case 0x81: f.bits       = v; break;
                case 0x82: f.channels   = v; break;
                case 0x83: f.codec      = v; break;
                case 0x84: f.sampleRate = v; break;
            }
        }
        f.valid = (p < end);
        return f.valid;
    }

    // 'SEAD' chunk - fixed format, payload ignored
    inline Format SeadFormat()
    {
        Format f; 
        f.valid = true; 
        f.bits = 16; 
        f.channels = 2; 
        f.sampleRate = 22050; 
        f.codec = eCodec_ImaSead;
        return f;
    }

    inline bool IsSupported(const Format& f)
    {
        if (!f.valid || ( f.bits != 16)) 
            return false;

        switch (f.codec)
        {
            case eCodec_Pcm16:
            case eCodec_EaAdpcm: 
                return (f.channels == 1) || (f.channels == 2);
            case eCodec_ImaSead: 
                return true;
        }
        return false;
    }

    class Decoder
    {
    public:
        // IMA SEAD keeps its predictor/step index between chunks of a track; call at start and on rewind
        inline void Reset() 
        { 
            for (int c = 0; c < 2; c++) 
            { 
                mPred[c] = 0; 
                mIndex[c] = 0; 
            } 
        }

        // appends interleaved s16 samples of one data chunk to 'out'
        // returns the number of sample FRAMES appended
        // (0 for unsupported/empty chunks)
        // never reads outside [p, p+size)
        uint32_t DecodeChunk(const Format& f, const uint8_t* p, uint32_t size, std::vector<int16_t>& out)
        {
            if (!IsSupported(f)) 
                return 0;
            switch (f.codec)
            {
                case eCodec_Pcm16:   return DecodePcm(f, p, size, out);
                case eCodec_EaAdpcm: return (f.channels == 2) ? DecodeEaStereo(p, size, out) : DecodeEaMono(p, size, out);
                case eCodec_ImaSead: return DecodeSead(p, size, out);
            }
            return 0;
        }

    private:
        static uint32_t DecodePcm(const Format& f, const uint8_t* p, uint32_t size, std::vector<int16_t>& out)
        {
            if (size < 4) 
                return 0;
            uint32_t frames = rl32(p);
            const uint32_t avail = (size - 4) / (2u * f.channels);
            if (frames > avail) 
            {
                frames = avail;
            }
            const uint32_t base = out.size();
            out.resize(base + frames * f.channels);
            const uint8_t* s = p + 4;
            for (uint32_t i = 0; i < frames * f.channels; i++)
            {
                out[base + i] = rl16s(s);
                s += 2;
            }
            return frames;
        }

        static uint32_t DecodeEaStereo(const uint8_t* p, uint32_t size, std::vector<int16_t>& out)
        {
            if (size < 12) 
                return 0;

            int currL = rl16s(p + 4);
            int prevL = rl16s(p + 6);
            int currR = rl16s(p + 8);
            int prevR = rl16s(p + 10);

            const uint8_t* src = p + 12;
            const uint32_t blocks = (size - 12) / 30;
            const uint32_t base = out.size();

            out.resize(base + (uint32_t)blocks * 28 * 2);

            int16_t* output_ptr = out.data() + base;

            for (uint32_t b = 0; b < blocks; b++, src += 30)
            {
                const int h0 = src[0];
                const int h1 = src[1];

                const int c1L = detail::EA_TABLE[h0 >> 4];
                const int c2L = detail::EA_TABLE[(h0 >> 4) + 4];
                const int c1R = detail::EA_TABLE[h0 & 15];
                const int c2R = detail::EA_TABLE[(h0 & 15) + 4];

                const int shL = (h1 >> 4) + 8;
                const int shR = (h1 & 15) + 8;

                for (int i = 0; i < 28; i++)
                {
                    const uint32_t by = src[2 + i];
                    const int vL = (int32_t)((by & 0xF0u) << 24) >> shL;
                    const int vR = (int32_t)(by << 28) >> shR;
                    const int nL = clip16((vL + currL * c1L + prevL * c2L + 0x80) >> 8);
                    const int nR = clip16((vR + currR * c1R + prevR * c2R + 0x80) >> 8);
                    prevL = currL; 
                    currL = nL; 
                    prevR = currR; 
                    currR = nR;
                    *output_ptr++ = (int16_t)nL; 
                    *output_ptr++ = (int16_t)nR;
                }
            }
            return blocks * 28;
        }

        static uint32_t DecodeEaMono(const uint8_t* p, uint32_t size, std::vector<int16_t>& out)
        {
            if (size < 12) 
                return 0;

            int curr = rl16s(p + 4);
            int prev = rl16s(p + 6);

            const uint8_t* src = p + 12;
            const uint32_t blocks = (size - 12) / 15;
            const uint32_t base = out.size();

            out.resize(base + (uint32_t)blocks * 28);

            int16_t* output_ptr = out.data() + base;

            for (uint32_t b = 0; b < blocks; b++, src += 15)
            {
                const int h = src[0];

                const int c1 = detail::EA_TABLE[h >> 4];
                const int c2 = detail::EA_TABLE[(h >> 4) + 4];

                const int sh = (h & 15) + 8;

                for (int i = 0; i < 14; i++)
                {
                    const uint32_t by = src[1 + i];
                    const int v0 = (int32_t)((by & 0xF0u) << 24) >> sh;
                    const int v1 = (int32_t)(by << 28) >> sh;

                    const int n0 = clip16((v0 + curr * c1 + prev * c2 + 0x80) >> 8);
                    prev = curr; 
                    curr = n0;

                    const int n1 = clip16((v1 + curr * c1 + prev * c2 + 0x80) >> 8);
                    prev = curr; 
                    curr = n1;

                    *output_ptr++ = (int16_t)n0; 
                    *output_ptr++ = (int16_t)n1;
                }
            }
            return blocks * 28;
        }

        inline uint32_t DecodeSead(const uint8_t* p, uint32_t size, std::vector<int16_t>& out)
        {
            const uint32_t base = out.size();

            out.resize(base + size * 2);

            int16_t* output_ptr = out.data() + base;

            for (uint32_t i = 0; i < size; i++)
            {
                const int by = p[i];
                *output_ptr++ = DecodeIma(0, by >> 4);
                *output_ptr++ = DecodeIma(1, by & 15);
            }
            return size;
        }

        inline int16_t DecodeIma(int c, int nib)
        {
            const int step = detail::IMA_STEP[mIndex[c]];
            int d = step >> 3;
            if (nib & 4) { d += step; }
            if (nib & 2) { d += step >> 1; }
            if (nib & 1) { d += step >> 2; }
            if (nib & 8) { d = -d; }
            mPred[c] = clip16(mPred[c] + d);
            int idx = mIndex[c] + detail::IMA_INDEX_DELTA[nib & 7];
            mIndex[c] = idx < 0 ? 0 : idx > 88 ? 88 : idx;
            return (int16_t)mPred[c];
        }

    private:
        int mPred[2] = { 0, 0 };
        int mIndex[2] = { 0, 0 };
    };

} // namespace dk2audio

//////////////////////////////////////////////////////////////////////////

bool DK2MovieDecoder::OpenFile(const std::string& path) 
{
    mFileStream.open(path, std::ios::binary);
    if (!mFileStream) 
    {
        mErrorCode = eErrorCode_FileError;
        return false;
    }

    mDataBuffer.reserve(1024 * 64);

    DemuxResult demuxNextResult;
    if (!DemuxNext(demuxNextResult) && (mErrorCode == eErrorCode_None)) 
    {
        mErrorCode = eErrorCode_EOF;
    }

    bool isSuccess = (mErrorCode == eErrorCode_None);
    if (isSuccess)
    {
        Rewind();
    }
    else
    {
        const eErrorCode errorCode = mErrorCode;
        Close();
        mErrorCode = errorCode;
    }
    return isSuccess;
}

void DK2MovieDecoder::Close()
{
    mFileStream.close();
    mDataBuffer.clear();

    mErrorCode = {};
    mFrameDims = {};
    mFrameDimsCoded = {};
    mFrameIndex = 0;
}

void DK2MovieDecoder::Rewind() 
{
    cxx_assert(mFileStream.is_open());
    if (mFileStream.is_open())
    {
        mFileStream.clear();
        mFileStream.seekg(0, std::ios::beg);
    }
    if (mErrorCode == eErrorCode_EOF)
    {
        mErrorCode = eErrorCode_None;
    }
    mFrameIndex = 0;
}

bool DK2MovieDecoder::DemuxNext(DemuxResult& outResult) 
{
    if (!mFileStream)
        return false;

    const uint32_t HeaderSize = 8;
    for (;;)
    {
        // chunk header
        uint8_t headerBytes[HeaderSize];
        if (!cxx::read_elements(mFileStream, headerBytes, HeaderSize))
            return false; // eof

        const uint32_t tag = rl32(headerBytes);
        const uint32_t raw = rl32(headerBytes + 4);
        if (raw < HeaderSize) 
        {
            mErrorCode = eErrorCode_DecodeError;
            return false;
        }

        if (tag == FOURCC_PIQT) 
        {
            mDataBuffer.clear();
            mDataBuffer.resize(raw - HeaderSize);

            if (!cxx::read_elements(mFileStream, mDataBuffer.data(), mDataBuffer.size()))
            {
                mErrorCode = eErrorCode_DecodeError;
                return false;
            }

            outResult.mTag = tag;
            outResult.mLength = mDataBuffer.size();
            outResult.mDataPtr = mDataBuffer.data();

            return true;
        }

        // unknown chunk (audio etc.) skip
        if (!mFileStream.seekg(raw - HeaderSize, std::ios::cur))
        {
            mErrorCode = eErrorCode_DecodeError;
            return false;
        }
    }
    return false;
}

bool DK2MovieDecoder::DecodeTqi(const uint8_t* payload, uint32_t size, ByteArray& outFrame) 
{
    if (size < 8) 
    {
        mErrorCode = eErrorCode_FrameTooSmall;
        return false;
    }

    tqi::FrameInfo fi;
    tqi::Decoder::readHeader(payload, size, fi);
    if (fi.width <= 0 || fi.height <= 0 || fi.width > 4096 || fi.height > 4096) 
    {
        mErrorCode = eErrorCode_InvalidFrameDimensions;
        return false;
    }

    mFrameDims = {fi.width, fi.height};
    mFrameDimsCoded.x = align16(fi.width);
    mFrameDimsCoded.y = align16(fi.height);

    outFrame.resize(fi.width * fi.height * 3);

    tqi::Decoder decoder;
    if (!decoder.decode(payload, size, outFrame.data(), (std::ptrdiff_t)fi.width * 3)) 
    {
        // bitstream ended before all macroblocks were decoded
        mErrorCode = eErrorCode_DecodeError;
        return false;
    }
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

    const bool isSuccess = DecodeTqi(demuxNextResult.mDataPtr, demuxNextResult.mLength, outFrame);
    if (isSuccess) 
    {
        mFrameIndex++;
    }
    return isSuccess;
}

bool DK2MovieDecoder::DecodeAudio(int track, AudioInfo& audioInfo, AudioData& audioData)
{
    audioInfo = {};
    audioData.clear();

    if ((track < 1) || !mFileStream.is_open())
    {
        cxx_assert(false);
        return false;
    }

    mFileStream.clear();
    const std::streampos savedPos = mFileStream.tellg();
    mFileStream.seekg(0, std::ios::end);
    const std::streamoff fileSize = mFileStream.tellg();
    mFileStream.seekg(0, std::ios::beg);

    const uint32_t HeaderSize = 8;

    // the file starts with one audio header chunk per track
    // after every video frame there is one data chunk per track: 
    // the n-th data chunk in a row belongs to track n
    // any other chunk starts the count over
    std::vector<dk2audio::Format> trackFormats;

    std::vector<uint8_t> chunkData;

    dk2audio::Decoder audioDecoder;

    int dataChunkNumber = 0;

    bool leadingHeaders = true;
    bool isFormatChecked = false;
    bool isFormatOk = false;

    std::streamoff chunkPos = 0;
    while (chunkPos + HeaderSize <= fileSize)
    {
        uint8_t headerBytes[HeaderSize];
        if (!cxx::read_elements(mFileStream, headerBytes, HeaderSize))
            break;

        const uint32_t tag = rl32(headerBytes);
        const uint32_t raw = rl32(headerBytes + 4);
        if ((raw < HeaderSize) || (chunkPos + raw > fileSize)) // damaged or truncated chunk
            break;

        const uint32_t payloadSize = raw - HeaderSize;
        chunkPos += raw;

        bool needPayload = false;
        if (dk2audio::IsHeaderTag(tag))
        {
            // headers that are not at the start of the file are only accepted if there were none before
            needPayload = leadingHeaders || trackFormats.empty();
            dataChunkNumber = 0;
        }
        else
        {
            leadingHeaders = false;
            if (dk2audio::IsDataTag(tag))
            {
                if (!isFormatChecked)
                {
                    isFormatChecked = true;
                    isFormatOk = ((uint32_t) track <= trackFormats.size()) && dk2audio::IsSupported(trackFormats[track - 1]);
                    if (!isFormatOk)
                        break;
                }
                dataChunkNumber++;
                needPayload = (dataChunkNumber == track) && isFormatOk;
            }
            else if (!dk2audio::IsOtherAudioTag(tag) || (tag == Fourcc('S','C','C','l')) || (tag == Fourcc('S','C','L','l')))
            {
                dataChunkNumber = 0; // video, unknown and counter chunks
            }
        }

        if (!needPayload)
        {
            if (!mFileStream.seekg(payloadSize, std::ios::cur))
                break;

            continue;
        }

        chunkData.resize(payloadSize);
        if ((payloadSize > 0) && !cxx::read_elements(mFileStream, chunkData.data(), chunkData.size()))
            break;

        if (dk2audio::IsHeaderTag(tag))
        {
            dk2audio::Format format;
            if (tag == Fourcc('S','E','A','D'))
            {
                format = dk2audio::SeadFormat();
            }
            else
            {
                dk2audio::ParseHeaderChunk(chunkData.data(), payloadSize, format);
            }
            trackFormats.push_back(format);
        }
        else
        {
            audioDecoder.DecodeChunk(trackFormats[track - 1], chunkData.data(), payloadSize, audioData);
        }
    }

    // restore pos
    mFileStream.clear();
    mFileStream.seekg(savedPos);

    if (audioData.empty() || ((uint32_t) track > trackFormats.size()))
    {
        return false;
    }

    const dk2audio::Format& format = trackFormats[track - 1];
    audioInfo.mSampleRate = format.sampleRate;
    audioInfo.mChannels = format.channels;

    if (format.channels > 0)
    {
        audioInfo.mFramesCount = audioData.size() / format.channels;
    }

    if (format.sampleRate > 0)
    {
        audioInfo.mDuration = (1.0f * audioInfo.mFramesCount / format.sampleRate);
    }
    return true;
}

#include "stdafx.h"
#include "DK2MovieDecoder.h"

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
 
        // colour tables
        struct ColorTables 
        {
            uint32_t a[512] {}, b[512] {}, c[512] {};     // Y, (U), (V) packed 9-bit R/G/B fields
            uint8_t  r[512] {}, g[512] {}, bl[512] {};    // field -> 8-bit channel (brightness 100%)

            constexpr ColorTables() 
            {
                for (int iroller = -256; iroller < 256; iroller++) 
                {
                    const int eax = clampi(iroller, -64, 63);
                    const uint32_t idx = static_cast<uint32_t>(iroller - 192) & 0x1ffu;
                    uint32_t ecx = static_cast<uint32_t>(eax) << 12; 
                    ecx = (ecx + static_cast<uint32_t>(eax)) << 11;
                    a[idx] = ecx + static_cast<uint32_t>(eax) + 0x4883d8a9u;

                    const double cc = static_cast<double>(eax) + 0.5;
                    const uint32_t v1 = static_cast<uint32_t>(0.5 - cc * 0.3441); // truncation toward zero == _ftol
                    const uint32_t v2 = static_cast<uint32_t>(0.5 - cc * -1.772);
                    const uint32_t v3 = static_cast<uint32_t>(0.5 - cc * 0.7141);
                    const uint32_t v4 = static_cast<uint32_t>(0.5 - cc * -1.402);

                    b[idx] = ((v1 & 0x1ff) << 11) + (v2 & 0x1ff);
                    c[idx] = ((v3 & 0x1ff)        + ((v4 & 0x1ff) << 12)) << 11;
                }
                for (int i = 0; i < 512; i++) 
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
 
        // bit reader: LE dwords, MSB first
        struct Bits 
        {
            const uint8_t* p; 
            uint32_t n; 
            uint64_t pos;

            inline uint32_t word(uint32_t w) const 
            {
                uint32_t o = w * 4; 
                uint32_t v = 0;
                if ((o + 4) <= n) 
                { 
                    std::memcpy(&v, p + o, 4); 
                    return fromLE(v); 
                }
                for (uint32_t i = 0; (i < 4) && ((o + i) < n); i++) 
                {
                    v |= static_cast<uint32_t>(p[o + i]) << (8 * i);
                }
                return v;
            }

            static uint32_t fromLE(uint32_t v) 
            {
        #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                return __builtin_bswap32(v);
        #else
                return v;
        #endif
            }

            inline uint32_t peek32() const 
            {
                const uint32_t w = static_cast<uint32_t>(pos >> 5); 
                const uint32_t s = pos & 31;
                const uint32_t a = word(w);
                return s ? (a << s) | (word(w + 1) >> (32 - s)) : a;
            }

            inline bool overrun() const { return pos > static_cast<uint64_t>(n) * 8; }
        };
 
        // IDCT
        constexpr double kA = 0.5411961078643799;
        constexpr double kB = 1.3065630197525024;
        constexpr double kC = 0.3826834261417389;
        constexpr int64_t kSq = 0x5a82799a;
 
        // "+1.5*2^52, read low dword" == round-to-nearest-even
        inline int32_t rne(double x) 
        {          
            double t = x + 6755399441055744.0; 
            uint64_t u; 
            std::memcpy(&u, &t, 8); 
            return static_cast<int32_t>(static_cast<uint32_t>(u));
        }

        inline void idct8(const int32_t* x, int32_t* o) 
        {
            const int32_t t35 = x[3] + x[5], A = x[5] - x[3], t17 = x[7] + x[1], B = x[1] - x[7];
            const int32_t S = t35 + t17, D = t17 - t35;
            const double Z = static_cast<double>(A + B) * kC;
            const int32_t Y1 = rne(static_cast<double>(A) * kA + Z), Y2 = rne(static_cast<double>(B) * kB - Z);
            const int32_t E  = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>((D * kSq) >> 32)) << 1);
            const int32_t r1 = Y1 + E, r2 = E + Y2, r3 = Y2 + S;
            const int32_t d26 = x[2] - x[6], s04 = x[4] + x[0], d04 = x[0] - x[4];
            const int32_t F  = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>((d26 * kSq) >> 32)) << 1);
            const int32_t s26 = x[6] + x[2] + F;
            const int32_t e0 = s26 + s04, e3 = s04 - s26, e1 = F + d04, e2 = d04 - F;
            o[0] = e0 + r3; o[7] = e0 - r3; o[1] = e1 + r2; o[6] = e1 - r2;
            o[2] = e2 + r1; o[5] = e2 - r1; o[3] = e3 + Y1; o[4] = e3 - Y1;
        }

        // sample -> colour-table index (s+256, clamped to the table range)
        inline uint16_t toIdx(int32_t s) 
        { 
            return static_cast<uint16_t>(clampi(s, -256, 255) + 256); 
        }
 
        inline void idctBlock(const int32_t* coef, uint16_t* out, int stride) 
        {
            int32_t tmp[8][8], o[8];
            for (int r = 0; r < 8; r++) 
            {
                const int32_t* in = coef + r * 8;
                if (!(in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7])) 
                { 
                    for (int j = 0; j < 8; j++) 
                    {
                        o[j] = in[0]; 
                    }
                }
                else 
                {
                    idct8(in, o);
                }
                for (int j = 0; j < 8; j++) 
                {
                    tmp[j][r] = o[j];
                }
            }
            for (int j = 0; j < 8; j++) 
            {
                idct8(tmp[j], o);
                for (int i = 0; i < 8; i++) 
                {
                    out[j * stride + i] = toIdx(o[i] >> 17);
                }
            }
        }
 
        inline int readDc(Bits& b, bool chroma) 
        {
            const uint32_t w = b.peek32();
            const uint8_t e = chroma ? TQI_DC_CHROMA[w >> 24] : TQI_DC_LUMA[w >> 25];
            int size = e >> 4; 
            b.pos += e & 15;
            if (!size) 
                return 0;
            uint32_t v = b.peek32() >> (32 - size); 
            b.pos += size;
            if (!((v >> (size - 1)) & 1)) 
            {
                v -= (1u << size) - 1;
            }
            return static_cast<int32_t>(v);
        }
 
        inline void decodeBlock(Bits& b, const int32_t* q, int& dcpred, bool chroma, uint16_t* out, int stride) 
        {
            int32_t coef[64] {};
            dcpred += readDc(b, chroma);
            coef[0] = static_cast<int32_t>(static_cast<uint32_t>(dcpred) * static_cast<uint32_t>(q[0]));
            const uint16_t* ac = acTable().e;
            int k = 0; 
            bool any = false;
            for (;;) 
            {
                uint32_t w = b.peek32() >> 16;
                uint16_t e = ac[w];
                if (e == 0)  // invalid code 
                    break; 
                int len = e >> 11, run = (e >> 6) & 31, lvl = e & 63;
                b.pos += len;
                if (lvl == 0) 
                {
                    if (run == 0) 
                        break; // EOB

                    uint32_t x = b.peek32() >> 18; b.pos += 14; // ESC: 6b run + 8b level
                    int a = x & 255; k += static_cast<int>((x >> 8) & 63) + 1;
                    if (a == 0)         { lvl = b.peek32() >> 24; b.pos += 8; }
                    else if (a == 0x80) { lvl = static_cast<int>(b.peek32() >> 24) - 256; b.pos += 8; }
                    else                  lvl = a > 0x80 ? a - 256 : a;
                } 
                else 
                {
                    k += run + 1;
                    if (b.peek32() >> 31) 
                    {
                        lvl = -lvl;
                    }
                    b.pos += 1;
                }
                if (k > 63) 
                    break;
                int pos = TQI_SCAN[k];
                coef[pos] = static_cast<int32_t>(static_cast<uint32_t>(lvl) * static_cast<uint32_t>(q[pos]));
                any = true;
            }
            // DC-only shortcut
            if (!any) 
            {    
                const uint16_t v = toIdx(coef[0] >> 17);
                for (int y = 0; y < 8; y++) 
                {
                    for (int x = 0; x < 8; x++) 
                    {
                        out[y * stride + x] = v;
                    }
                }
            } 
            else 
            {
                idctBlock(coef, out, stride);
            }
        }
 
        inline void put(uint8_t* d, unsigned r, unsigned g, unsigned b) 
        {
            d[0] = r; 
            d[1] = g; 
            d[2] = b;
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
            const auto& T = detail::kColor;
            detail::Bits b{p + 8, n - 8, 0};
            int dc[3] = {0, 0, 0};
            const int mbw = (fi.width + 15) / 16, mbh = (fi.height + 15) / 16;
            uint16_t Y[16 * 16], U[8 * 8], V[8 * 8];
            for (int my = 0; my < mbh; my++) 
            {
                for (int mx = 0; mx < mbw; mx++) 
                {
                    detail::decodeBlock(b, q_, dc[0], false, Y,              16);
                    detail::decodeBlock(b, q_, dc[0], false, Y + 8,          16);
                    detail::decodeBlock(b, q_, dc[0], false, Y + 8 * 16,     16);
                    detail::decodeBlock(b, q_, dc[0], false, Y + 8 * 16 + 8, 16);
                    detail::decodeBlock(b, q_, dc[1], true,  U, 8);
                    detail::decodeBlock(b, q_, dc[2], true,  V, 8);
                    const int x0 = mx * 16, y0 = my * 16;
                    const int cw = fi.width - x0 < 16 ? fi.width - x0 : 16;
                    const int ch = fi.height - y0 < 16 ? fi.height - y0 : 16;
                    for (int y = 0; y < ch; y++) 
                    {
                        uint8_t* d = dst + static_cast<std::ptrdiff_t>(y0 + y) * stride + static_cast<std::ptrdiff_t>(x0) * bytesPerPixel;
                        const uint16_t* yy = Y + y * 16; 
                        const uint16_t* uu = U + (y >> 1) * 8; 
                        const uint16_t* vv = V + (y >> 1) * 8;
                        for (int x = 0; x < cw; x++, d += bytesPerPixel) 
                        {
                            uint32_t s = T.a[yy[x]] + T.b[uu[x >> 1]] + T.c[vv[x >> 1]];
                            detail::put(d, T.r[s >> 23], T.g[(s & 0xff800) >> 11], T.bl[s & 0x1ff]);
                        }
                    }
                }
            } // for
            return !b.overrun();
        }
    };
 
}  // namespace tqi

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

// bitstream: TGQ = LE, bits taken LSB-first
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

// EA IDCT (same integer kernel as eaidct.c)
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
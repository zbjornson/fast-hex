#include "hex.h"

#if defined(__GNUC__) // GCC, clang
#ifdef __clang__
#if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 4)
#error("Requires clang >= 3.4")
#endif // clang >=3.4
#else
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#error("Requires GCC >= 4.8")
#endif // gcc >=4.8
#endif // __clang__

#include <immintrin.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#endif

// ASCII -> hex value
static const uint8_t unhex_table[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

// ASCII -> hex value << 4 (upper nibble)
static const uint8_t unhex_table4[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   0, 16, 32, 48, 64, 80, 96,112,128,144, -1, -1, -1, -1, -1, -1,
  -1,160,176,192,208,224,240, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1,160,176,192,208,224,240, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

// Looks up the value for the lower nibble.
static inline int8_t unhexB(uint8_t x) { return unhex_table[x]; }

// Looks up the value for the upper nibble. Equivalent to `unhexB(x) << 4`.
static inline int8_t unhexA(uint8_t x) { return unhex_table4[x]; }

static inline int8_t unhexBitManip(uint8_t x) {
  return 9 * (x >> 6) + (x & 0xf);
}

static const __m256i _9 = _mm256_set1_epi16(9);
static const __m256i _15 = _mm256_set1_epi16(0xf);

inline static __m256i unhexBitManip(const __m256i value) {
  __m256i and15 = _mm256_and_si256(value, _15);

#ifndef NO_MADDUBS
  __m256i sr6 = _mm256_srai_epi16(value, 6);
  __m256i mul = _mm256_maddubs_epi16(sr6, _9); // this has a latency of 5
#else
  // ... while this I think has a latency of 4, but worse throughput(?).
  // (x >> 6) * 9 is x * 8 + x:
  // ((x >> 6) << 3) + (x >> 6)
  // We need & 0b11 to emulate 8-bit operations (narrowest shift is 16b) -- or a left shift
  // (((x >> 6) & 0b11) << 3) + ((x >> 6) & 0b11)
  // or
  // tmp = (x >> 6) & 0b11
  // tmp << 3 + tmp
  // there's no carry due to the mask+shift combo, so + is |
  // tmp << 3 | tmp
  __m256i sr6_lo2 = _mm256_and_si256(_mm256_srli_epi16(value, 6), _mm256_set1_epi16(0b11));
  __m256i sr6_lo2_sl3 = _mm256_slli_epi16(sr6_lo2, 3);
  __m256i mul = _mm256_or_si256(sr6_lo2_sl3, sr6_lo2);
#endif

  __m256i add = _mm256_add_epi16(mul, and15);
  return add;
}

static const char hex_table[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};
inline static char hex(uint8_t value) { return hex_table[value]; }
static const __m256i HEX_LUTR = _mm256_setr_epi8(
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f');
inline static __m256i hex(__m256i value) {
  return _mm256_shuffle_epi8(HEX_LUTR, value);
}

// (a << 4) | b;
// a and b must be 16-bit elements. Output is packed 8-bit elements.
inline static __m256i nib2byte(__m256i a1, __m256i b1, __m256i a2, __m256i b2) {
  __m256i a4_1 = _mm256_slli_epi16(a1, 4);
  __m256i a4_2 = _mm256_slli_epi16(a2, 4);
  __m256i a4orb_1 = _mm256_or_si256(a4_1, b1);
  __m256i a4orb_2 = _mm256_or_si256(a4_2, b2);
  __m256i pck1 = _mm256_packus_epi16(a4orb_1, a4orb_2); // lo1 lo2 hi1 hi2
  const int _0213 = 0b11'01'10'00;
  __m256i pck64 = _mm256_permute4x64_epi64(pck1, _0213);
  return pck64;
}

static const __m256i ROT2 = _mm256_setr_epi8(
  -1, 0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14,
  -1, 0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14
);
// a -> [a >> 4, a & 0b1111]
inline static __m256i byte2nib(__m128i val) {
  __m256i doubled = _mm256_cvtepu8_epi16(val);
  __m256i hi = _mm256_srli_epi16(doubled, 4);
  __m256i lo = _mm256_shuffle_epi8(doubled, ROT2);
  __m256i bytes = _mm256_or_si256(hi, lo);
  bytes = _mm256_and_si256(bytes, _mm256_set1_epi8(0b1111));
  return bytes;
}

// len is number or dest bytes (i.e. half of src length)
void decodeHexBMI(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t a = *src++;
    uint8_t b = *src++;
    a = unhexBitManip(a);
    b = unhexBitManip(b);
    dest[i] = (a << 4) | b;
  }
}

// len is number or dest bytes (i.e. half of src length)
void decodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  const __m256i A_MASK = _mm256_setr_epi8(
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1,
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1);
  const __m256i B_MASK = _mm256_setr_epi8(
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1,
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1);

  const __m256i* val3 = reinterpret_cast<const __m256i*>(src);
  __m256i* dec256 = reinterpret_cast<__m256i*>(dest);

  while (len >= 32) {
    __m256i av1 = _mm256_lddqu_si256(val3++); // 32 nibbles, 16 bytes
    __m256i av2 = _mm256_lddqu_si256(val3++);
                                                // Separate high and low nibbles and extend into 16-bit elements
    __m256i a1 = _mm256_shuffle_epi8(av1, A_MASK);
    __m256i b1 = _mm256_shuffle_epi8(av1, B_MASK);
    __m256i a2 = _mm256_shuffle_epi8(av2, A_MASK);
    __m256i b2 = _mm256_shuffle_epi8(av2, B_MASK);

    // Convert ASCII values to nibbles
    a1 = unhexBitManip(a1);
    a2 = unhexBitManip(a2);
    b1 = unhexBitManip(b1);
    b2 = unhexBitManip(b2);

    // Nibbles to bytes
    __m256i bytes = nib2byte(a1, b1, a2, b2);

    _mm256_storeu_si256(dec256++, bytes);
    len -= 32;
  }

  src = reinterpret_cast<const uint8_t*>(val3);
  dest = reinterpret_cast<uint8_t*>(dec256);
  decodeHexBMI(dest, src, len);
}

// len is number of dest bytes
void decodeHexLUT(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t a = *src++;
    uint8_t b = *src++;
    a = unhexB(a);
    b = unhexB(b);
    dest[i] = (a << 4) | b;
  }
}

// len is number of dest bytes
void decodeHexLUT4(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t a = *src++;
    uint8_t b = *src++;
    a = unhexA(a);
    b = unhexB(b);
    dest[i] = a | b;
  }
}

// len is number of src bytes
void encodeHex(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t a = src[i];
    uint8_t lo = a & 0b1111;
    uint8_t hi = a >> 4;
    *dest++ = hex(hi);
    *dest++ = hex(lo);
  }
}

// len is number of src bytes
void encodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  const __m128i* input128 = reinterpret_cast<const __m128i*>(src);
  __m256i* output256 = reinterpret_cast<__m256i*>(dest);

  size_t tailLen = len % 16;
  size_t vectLen = (len - tailLen) >> 4;
  for (size_t i = 0; i < vectLen; i++) {
    __m128i av = _mm_lddqu_si128(&input128[i]);
    __m256i nibs = byte2nib(av);
    __m256i hexed = hex(nibs);
    _mm256_storeu_si256(&output256[i], hexed);
  }

  encodeHex(dest + (vectLen << 5), src + (vectLen << 4), tailLen);
}

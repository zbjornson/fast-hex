#ifndef SPEEDILY_HEX_
#define SPEEDILY_HEX_

#include <stdint.h>

#if defined(_MSC_VER)
#include <intrin.h>
#define __restrict__ __restrict  // The C99 keyword, available as a C++ extension
#endif

// Scalar look-up table version. len is length of output buffer.
void decodeHexLUT(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// Optimized scalar look-up table version (avoids a shift). len is length of output buffer.
void decodeHexLUT4(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// Optimal AVX2 vectorized version.
void decodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// Scalar version. len is number of src bytes.
void encodeHex(uint8_t* __restrict__ dest, uint8_t* __restrict__ src, size_t len);

// AVX2 vectorized version. len is number of src bytes.
void encodeHexVec(uint8_t* __restrict__ dest, uint8_t* __restrict__ src, size_t len);

#endif

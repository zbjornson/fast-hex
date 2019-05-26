#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
#define __restrict__ __restrict  // The C99 keyword, available as a C++ extension
#endif

// Decoders
// Decode src hex string into dest bytes

// Scalar look-up table version. len is number of dest bytes (1/2 the size of src).
void decodeHexLUT(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// Optimized scalar look-up table version (avoids a shift). len is number of dest bytes (1/2 the size of src).
void decodeHexLUT4(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// Optimal AVX2 vectorized version. len is number of dest bytes (1/2 the size of src).
void decodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// Encoders
// Encode src bytes into dest hex string

// Scalar version. len is number of src bytes. dest must be twice the size of src.
void encodeHex(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

// AVX2 vectorized version. len is number of src bytes. dest must be twice the size of src.
void encodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

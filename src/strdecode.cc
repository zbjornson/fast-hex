#include <nan.h>
#include <stdint.h>
#include <iostream>

#ifdef PROFILE_
#include <chrono>
#include <time.h>
#endif

using namespace v8;

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
#define __restrict__ __restrict  // The C99 keyword, available as a C++ extension
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

static const __m256i _9 = _mm256_set1_epi16(9);
static const __m256i _15 = _mm256_set1_epi16(0xf);

// Looks up the value for the lower nibble.
static inline int8_t unhexB(uint8_t x) { return unhex_table[x]; }

// Looks up the value for the upper nibble. Equivalent to `unhexB(x) << 4`.
static inline int8_t unhexA(uint8_t x) { return unhex_table4[x]; }

// Calculates hex value 
static inline int8_t unhexBitManip(uint8_t x) { return 9 * (x >> 6) + (x & 0xf); }
inline static __m256i unhexBitManip(__m256i value) {
  __m256i sr6 = _mm256_srai_epi16(value, 6);
  __m256i and15 = _mm256_and_si256(value, _15);
  __m256i mul = _mm256_maddubs_epi16(sr6, _9);
  __m256i add = _mm256_add_epi16(mul, and15);
  return add;
}

// (a << 4) | b;
// a and b must be 16-bit elements. Output is packed 8-bit elements.
inline static __m128i nib2byte(__m256i a, __m256i b) {
  __m256i a4 = _mm256_slli_epi16(a, 4);
  __m256i a4orb = _mm256_or_si256(a4, b);
  __m256i pck1 = _mm256_packus_epi16(a4orb, a4orb);
  const int _0200 = 0b00001000;
  __m256i pck64 = _mm256_permute4x64_epi64(pck1, _0200);
  return _mm256_castsi256_si128(pck64);
}
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

// Scalar bit-manipulation version.
// len is length of output buffer
void decodeHexBMI(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  size_t j = 0;
  for (size_t i = 0; i < len; i++) {
    uint8_t a = src[j++];
    uint8_t b = src[j++];
    a = unhexBitManip(a);
    b = unhexBitManip(b);
    dest[i] = (a << 4) | b;
  }
}

// Optimal(?) vectorized version.
// len is length of output buffer.
void decodeHexVec4(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  const __m256i A_MASK = _mm256_setr_epi8(
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1,
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1);
  const __m256i B_MASK = _mm256_setr_epi8(
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1,
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1);

  const __m256i* val3 = reinterpret_cast<const __m256i*>(src);
  __m128i* dec128 = reinterpret_cast<__m128i*>(dest);

  size_t tailLen = len % 16;
  size_t vectLen = (len - tailLen) >> 4;
  size_t i = 0;
  while (i < vectLen) {
    __m256i av = _mm256_lddqu_si256(&val3[i]); // 32 nibbles, 16 bytes

    // Separate high and low nibbles and extend into 16-bit elements
    __m256i av1 = _mm256_shuffle_epi8(av, A_MASK);
    __m256i av2 = _mm256_shuffle_epi8(av, B_MASK);

    // Convert ASCII values to nibbles
    av1 = unhexBitManip(av1);
    av2 = unhexBitManip(av2);

    // Nibbles to bytes
    __m128i bytes = nib2byte(av1, av2);

    _mm_store_si128(&dec128[i], bytes);
    i++;
  }

  decodeHexBMI(dest + (vectLen << 4), src + (vectLen << 5), tailLen);
}

void decodeHexVec8(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  const __m256i A_MASK = _mm256_setr_epi8(
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1,
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1);
  const __m256i B_MASK = _mm256_setr_epi8(
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1,
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1);

  const __m256i* val3 = reinterpret_cast<const __m256i*>(src);
  __m256i* dec256 = reinterpret_cast<__m256i*>(dest);

  size_t tailLen = len % 32;
  size_t vectLen = (len - tailLen) >> 5;
  size_t i = 0, j = 0;
  while (i < vectLen) {
    __m256i av1 = _mm256_lddqu_si256(&val3[i++]); // 32 nibbles, 16 bytes
    __m256i av2 = _mm256_lddqu_si256(&val3[i++]);
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

    _mm256_store_si256(&dec256[j++], bytes);
  }

  decodeHexBMI(dest + (vectLen << 5), src + (vectLen << 6), tailLen);
}

// Simple look-up table version.
// len is length of output buffer
void decodeHexLUT(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  size_t j = 0;
  for (size_t i = 0; i < len; i++) {
    uint8_t a = src[j++];
    uint8_t b = src[j++];
    a = unhexB(a);
    b = unhexB(b);
    dest[i] = (a << 4) | b;
  }
}

// Optimized look-up table version (avoids a shift).
// len is length of output buffer
void decodeHexLUT4(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  size_t j = 0;
  for (size_t i = 0; i < len; i++) {
    uint8_t a = src[j++];
    uint8_t b = src[j++];
    a = unhexA(a);
    b = unhexB(b);
    dest[i] = a | b;
  }
}

// Returns true if data was allocated and should be freed (with _mm_free)
bool bytesFromString(Local<Value> val, const uint8_t** data, size_t* length) {
  if (node::Buffer::HasInstance(val)) {
    *data = reinterpret_cast<uint8_t*>(node::Buffer::Data(val));
    *length = node::Buffer::Length(val);
    return false;
  }

  if (!val->IsString()) return false;

  Local<String> str = val.As<String>();
  if (str->IsExternalOneByte()) {
    //std::cout << "external one byte" << std::endl;
    const String::ExternalOneByteStringResource* ext = str->GetExternalOneByteStringResource();
    *data = (const uint8_t*)ext->data();
    *length = ext->length();
    return false;
  } else if (str->IsOneByte()) {
    //std::cout << "internal one byte" << std::endl;
    *length = str->Length();
    *data = (const uint8_t*)_mm_malloc(*length, 64);
    str->WriteOneByte(const_cast<uint8_t*>(*data));
    return true;
  } else {
    std::cout << "external 2-byte string encountered" << std::endl;
    return false;
  }
}

template <int METHOD>
NAN_METHOD(decodeHex) {
#ifdef PROFILE_
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  std::chrono::nanoseconds time_span;
  start = std::chrono::high_resolution_clock::now();
#endif

  const uint8_t* value;
  size_t inLen;
  bool needsFree = bytesFromString(info[1], &value, &inLen);

  Local<Uint8Array> destTa = info[0].As<Uint8Array>();
  size_t outLen = inLen >> 1;
  Nan::TypedArrayContents<uint8_t> decoded(destTa);


#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
  start = std::chrono::high_resolution_clock::now();
#endif

  if (METHOD == 1) decodeHexVec4(*decoded, value, outLen);
  if (METHOD == 2) decodeHexLUT(*decoded, value, outLen);
  if (METHOD == 3) decodeHexLUT4(*decoded, value, outLen);
  if (METHOD == 4) decodeHexVec8(*decoded, value, outLen);
  
#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
#endif

  if (needsFree) _mm_free((void*)value);
  //Local<v8::Object> buf = Nan::NewBuffer(decoded, bufLength).ToLocalChecked();
  //info.GetReturnValue().Set(buf);
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("decodeHexVec").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<1>)).ToLocalChecked());
  Nan::Set(target, Nan::New("decodeHexNode").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<2>)).ToLocalChecked());
  Nan::Set(target, Nan::New("decodeHexNode2").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<3>)).ToLocalChecked());
  Nan::Set(target, Nan::New("decodeHexVec8").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<4>)).ToLocalChecked());
}

NODE_MODULE(strdecode, Init);

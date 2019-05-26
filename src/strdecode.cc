#include <nan.h>
#include <stdint.h>
#include <iostream>
#include "hex.h"

#if defined(__GNUC__) // GCC, clang
#include <x86intrin.h> // _mm_malloc
#elif defined(_MSC_VER)
#include <intrin.h>
#endif

#ifdef PROFILE_
#include <chrono>
#include <time.h>
#endif

using namespace v8;

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

  if (METHOD == 0) decodeHexLUT(*decoded, value, outLen);
  if (METHOD == 1) decodeHexLUT4(*decoded, value, outLen);
  if (METHOD == 2) decodeHexVec(*decoded, value, outLen);

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
#endif

  if (needsFree) _mm_free((void*)value);
  //Local<v8::Object> buf = Nan::NewBuffer(decoded, bufLength).ToLocalChecked();
  //info.GetReturnValue().Set(buf);
}

template <int METHOD>
NAN_METHOD(encodeHex) {
  EscapableHandleScope scope(Isolate::GetCurrent());
  
  Local<Uint8Array> srcTa = info[0].As<Uint8Array>();
  Nan::TypedArrayContents<uint8_t> src(srcTa);
  size_t srcLen = srcTa->Length();

  size_t destLen = srcLen << 1;
  uint8_t* dst = (uint8_t*)_mm_malloc(destLen, 64);

#ifdef PROFILE_
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  std::chrono::nanoseconds time_span;
  start = std::chrono::high_resolution_clock::now();
#endif

  if (METHOD == 0) encodeHex(dst, *src, srcLen);
  if (METHOD == 1) encodeHexVec(dst, *src, srcLen);

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
  start = std::chrono::high_resolution_clock::now();
#endif

  Local<String> str = v8::String::NewFromOneByte(
    Isolate::GetCurrent(),
    dst,
    v8::NewStringType::kNormal,
    destLen).ToLocalChecked();

#ifdef PROFILE_
  end = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << time_span.count() << " ns\n";
#endif

  _mm_free(dst);

  // TODO use external strings above apex

  info.GetReturnValue().Set(scope.Escape(str));
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("decodeHexNode").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<0>)).ToLocalChecked());
  Nan::Set(target, Nan::New("decodeHexNode2").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<1>)).ToLocalChecked());
  Nan::Set(target, Nan::New("decodeHexVec").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(decodeHex<2>)).ToLocalChecked());

  Nan::Set(target, Nan::New("encodeHex").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(encodeHex<0>)).ToLocalChecked());
  Nan::Set(target, Nan::New("encodeHexVec").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(encodeHex<1>)).ToLocalChecked());
}

NODE_MODULE(strdecode, Init);

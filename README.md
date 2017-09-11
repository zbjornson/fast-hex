# Speedily

Fast replacements for node.js core modules on x86/64 (early work in progress).

Node.js supports a [variety of architectures](https://github.com/nodejs/node/blob/master/BUILDING.md#supported-platforms-1),
including x86/64, ARM, ARM64, s390x and PPC64le/be. The modules here are much
faster by means of manual vectorization (SIMD), but only run on recent x86/64
processors.

This is an early work in progress. Near-term goal is to provide both direct
access to the low-level accelerated functions, such as:

    speedily.decodeHex(Uint8Array|Buffer output, Uint8Array|Buffer input)

and to provide node.js API-compatible interfaces that automatically use the
accelerated functions when possible and revert to the stock implementation otherwise,
such as:

    speedily.Buffer.from(string, encoding)
    // supported encodings will be accelerated
    // unsupported encodings wlll drop down to node's stock implementation
    // optionally use global.Buffer = speedily.Buffer for global acceleration

(Unclear yet how many APIs can be modified like this.)

---

### Decoding strings/buffers

    speedily.decodeHexVec8(Uint8Array|Buffer output, Uint8Array|Buffer|String input)

**Acclerates**: `Buffer.from(string, "hex")`. Maybe in the future: `buffer.transcode()`,
`StringDecoder`, `stream.write(string, "hex")`.

**Benchmark**: ~2.12x faster for short string inputs (< 1,031,913 chars), ~12x faster
for some long strings, Buffers and TypedArrays.

**Requires**: AVX2 (Haswell or later) currently. (AVX or maybe earlier possible.)

Development notes:
* Accessing string bytes from v8 is slow. Node uses external strings in some cases,
  but so far the only scenarios in which I've found them used are (a) via `buffer.toString()`,
  (b) via the C++ API
  `v8::Local<v8::Value> node::Encode(v8::Isolate* isolate, const char* buf, size_t len, enum encoding encoding)`.
  Haven't looked yet at what cases cause v8 to automatically use external strings.
* `stream.write` uses `StringBytes::Encode` internally. Can't replace this without
  modifying node.js, unless we intercept `stream.write` in JS.

### Byte-swapping: `buffer.swap16/32/64()`

See [node-bswap](https://github.com/zbjornson/node-bswap) (up to ~12x faster; SSSE3 or AVX2).

---

### Other info

Supported compilers:
* MSVC 2015 or later, or Visual C++ Build Tools 2015 or later
* Clang 3.4.x or later
* GCC 4.8.x or later
* ICC 16 or later

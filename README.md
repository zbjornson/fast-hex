# Speedily

Ultrafast string encoders/decoders for x86/64, available as stand-alone C++
modules and as node.js modules (early work in progress). See notes for
individual functions for CPU requirements; most require AVX2 (Haswell or later).

## Usage in Node.js

Node.js supports a [variety of architectures](https://github.com/nodejs/node/blob/master/BUILDING.md#supported-platforms-1),
including x86/64, ARM, ARM64, s390x and PPC64le/be. These functions are faster,
but only work on x86/64. Sometime I might try to get them integrated into
node.js core, but doing so would likely require them to change their test
strategy in order to maintain test coverage.

This is an early work in progress. Near-term goal is to provide both direct
access to the low-level accelerated functions, such as:

```typescript
speedily.decodeHexVec(output: Uint8Array|Buffer, input: Uint8Array|Buffer)
```

and to (optionally) patch node.js's modules at runtime to automatically use the
accelerated functions when possible, and revert to the stock implementation
otherwise, such as:

```typescript
speedily.Buffer.from(string, encoding)
// supported encodings will be accelerated
// unsupported encodings wlll drop down to node's stock implementation
// optionally use global.Buffer = speedily.Buffer for global acceleration?
```

(Unclear yet how many APIs can be modified like this.)

## Usage in C++

Include the header appropriate header file from `src/` and adjust your build
appropriately (GCC/Clang/ICC: `-march=haswell` for example; MSVC: set the
EnableAdvancedInstructionSet to "AVX2" or `/arch:AVX2`).

---

## Hex string encoding/decoding

```typescript
speedily.decodeHexVec(output: Uint8Array|Buffer, input: Uint8Array|Buffer): void;
speedily.encodeHexVec(input: Uint8Array|Buffer): string;
```

| | |
| -- | -- |
| Benchmark | <b>Node</b> <li> ~2.12x faster decoding for short string inputs (< 1,031,913 chars) <li> ~12x faster decoding for some long strings, Buffers and TypedArrays <li> ~5.5x faster encoding <br><b>C++</b> <li> Decoding: ~12.5x over scalar <li> Encoding: ~11.5x over scalar |
| Accelerates | <li> `Buffer.from(string, "hex")` <li> `Buffer.toString("hex")` <li> Maybe in the future: `buffer.transcode()`, `StringDecoder`, `stream.write(string, "hex")`.
| Requires | AVX2 (Haswell or later) currently. (AVX or maybe earlier possible.)

Development notes:
* Accessing string bytes from v8 is slow. Node uses external strings in some
  cases, but so far the only scenarios in which I've found them used are (a)
  via `buffer.toString()`, (b) via the C++ API
  `v8::Local<v8::Value> node::Encode(v8::Isolate* isolate, const char* buf, size_t len, enum encoding encoding)`.
  Haven't looked yet at what cases cause v8 to automatically use external
  strings.
* `stream.write` uses `StringBytes::Encode` internally. Can't replace this
  without modifying node.js, unless we intercept `stream.write` in JS.
* Creating strings is also expensive. Still need to look at if there's a better
  way to do this, and definitely need to use external strings over some length threshold.

## Byte-swapping: `buffer.swap16/32/64()`

See [node-bswap](https://github.com/zbjornson/node-bswap) (up to ~12x faster; SSSE3 or AVX2).

---

### Other info

Supported compilers:
* MSVC 2015 or later, or Visual C++ Build Tools 2015 or later
* Clang 3.4.x or later
* GCC 4.8.x or later
* ICC 16 or later

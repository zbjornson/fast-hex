const pkg = require(".");
const assert = require("assert");

const inp = Buffer.from([
	128,0,48,192,47,52,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,08,0,0,0,156,
	1,0,0,140,2,0,0,0,
	0,0,0,0,0,32,0,00,
	96,46,99
]);

const a = pkg.encodeHex(inp);
const b = pkg.encodeHexVec(inp);

assert.deepStrictEqual(a, b);
console.log("Encoding OK");

const dest1 = new Uint8Array(a.length / 2);
const dest2 = new Uint8Array(a.length / 2);
const dest3 = new Uint8Array(a.length / 2);

pkg.decodeHexNode(dest1, a);
pkg.decodeHexNode2(dest2, a);
pkg.decodeHexVec(dest3, a);
assert.deepStrictEqual(dest1, dest2);
assert.deepStrictEqual(dest1, dest3);
console.log("Decoding OK");

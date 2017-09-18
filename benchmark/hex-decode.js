var Benchmark = require("benchmark");
var chalk = require("chalk");
var strdecode = require("..");

// From benchmark.js
function formatNumber(number) {
	number = String(number).split('.');
	return number[0].replace(/(?=(?:\d{3})+$)(?!\b)/g, ',') +
	(number[1] ? '.' + number[1] : '');
}

function padLeft(input, size) {
	return Array(size - input.length + 1).join(" ") + input;
}

// Modified from benchmark.js
function formatResult(event, times) {
	var hz = event.hz * times;
	var stats = event.stats;
	var size = stats.sample.length;
	var pm = '\xb1';

	var result = " (array length " + times + ")";

	result += ' x ' + chalk.cyan(formatNumber(hz.toFixed(hz < 100 ? 2 : 0))) + ' ops/sec ' + pm +
		stats.rme.toFixed(2) + '% (' + size + ' run' + (size == 1 ? '' : 's') + ' sampled)';

	return result;
}

const stringLengths = [/*2, 10, 100, 1000, 1600, */ 1600000 << 1];

const methods = [
	{label: "node", fn: strdecode.decodeHexNode},
	{label: "preshift", fn: strdecode.decodeHexNode2},
	{label: "avx2", fn: strdecode.decodeHexVec},
	{label: "bmi", fn: strdecode.decodeHexBMI}
];

const randomHexChar = (function () {
	const allowed = "abcdefABCDEF0123456789".split("");
	const nAllowed = allowed.length;
	return function () {
		return allowed[Math.floor(Math.random() * nAllowed)];
	};
})();

console.log(
	padLeft("strlen", 10),
	...methods.map(v => padLeft(v.label, 15))
);

for (var s = 0; s < stringLengths.length; s++) {
	var stringLength = stringLengths[s];
	var receiver = new Uint8Array(stringLength << 1);

	var input = ""
	for (var i = 0; i < stringLength; i++) input += randomHexChar();
	input = Buffer.from(input, "utf-8").asciiSlice(0);

	// Accuracy tests:
	// var receiver1 = Buffer.alloc(stringLength << 1);
	// var receiver2 = Buffer.alloc(stringLength << 1);
	// strdecode.decodeHexNode(receiver1, input);
	// strdecode.decodeHexVec(receiver2, input);
	// console.log(receiver1.equals(receiver2));
	// continue;

	var results = {};

	for (var m = 0; m < methods.length; m++) {
		var method = methods[m];

		new Benchmark.Suite()
			.add({
				name: method.label,
				fn: function () {
					method.fn(receiver, input);
				}
			})
			.on("cycle", function (event) {
				results[method.label] = event.target.hz;
			})
			.run();
	}

	console.log(
		padLeft(String(stringLength), 10),
		...methods.map(v => padLeft(formatNumber(results[v.label].toFixed(0)), 15))
	);
}

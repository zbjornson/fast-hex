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

const bufferLengths = [/*2, 10, 100, 1000,*/ 1600, 16000, 1600000];

const methods = [
	{label: "node", fn: (inp) => { inp.toString("hex"); }},
	{label: "scalar", fn: strdecode.encodeHex},
	{label: "avx2", fn: strdecode.encodeHexVec}
];

console.log(
	padLeft("strlen", 10),
	...methods.map(v => padLeft(v.label, 15))
);

for (var s = 0; s < bufferLengths.length; s++) {
	var bufferLength = bufferLengths[s];

	var input = Buffer.alloc(bufferLength);
	for (var i = 0; i < bufferLength; i++) Math.random() * 255;

	var results = {};

	for (var m = 0; m < methods.length; m++) {
		var method = methods[m];

		new Benchmark.Suite()
			.add({
				name: method.label,
				fn: function () {
					method.fn(input);
				}
			})
			.on("cycle", function (event) {
				results[method.label] = event.target.hz;
			})
			.run();
	}

	console.log(
		padLeft(String(bufferLength), 10),
		...methods.map(v => padLeft(formatNumber(results[v.label].toFixed(0)), 15))
	);
}

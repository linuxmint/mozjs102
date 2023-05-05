// Copyright 2021 Mathias Bynens. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
author: Mathias Bynens
description: >
  Unicode property escapes for `Script=Vithkuqi`
info: |
  Generated by https://github.com/mathiasbynens/unicode-property-escapes-tests
  Unicode v14.0.0
esid: sec-static-semantics-unicodematchproperty-p
features: [regexp-unicode-property-escapes]
includes: [regExpUtils.js]
---*/

const matchSymbols = buildString({
  loneCodePoints: [],
  ranges: [
    [0x010570, 0x01057A],
    [0x01057C, 0x01058A],
    [0x01058C, 0x010592],
    [0x010594, 0x010595],
    [0x010597, 0x0105A1],
    [0x0105A3, 0x0105B1],
    [0x0105B3, 0x0105B9],
    [0x0105BB, 0x0105BC]
  ]
});
testPropertyEscapes(
  /^\p{Script=Vithkuqi}+$/u,
  matchSymbols,
  "\\p{Script=Vithkuqi}"
);
testPropertyEscapes(
  /^\p{Script=Vith}+$/u,
  matchSymbols,
  "\\p{Script=Vith}"
);
testPropertyEscapes(
  /^\p{sc=Vithkuqi}+$/u,
  matchSymbols,
  "\\p{sc=Vithkuqi}"
);
testPropertyEscapes(
  /^\p{sc=Vith}+$/u,
  matchSymbols,
  "\\p{sc=Vith}"
);

const nonMatchSymbols = buildString({
  loneCodePoints: [
    0x01057B,
    0x01058B,
    0x010593,
    0x010596,
    0x0105A2,
    0x0105B2,
    0x0105BA
  ],
  ranges: [
    [0x00DC00, 0x00DFFF],
    [0x000000, 0x00DBFF],
    [0x00E000, 0x01056F],
    [0x0105BD, 0x10FFFF]
  ]
});
testPropertyEscapes(
  /^\P{Script=Vithkuqi}+$/u,
  nonMatchSymbols,
  "\\P{Script=Vithkuqi}"
);
testPropertyEscapes(
  /^\P{Script=Vith}+$/u,
  nonMatchSymbols,
  "\\P{Script=Vith}"
);
testPropertyEscapes(
  /^\P{sc=Vithkuqi}+$/u,
  nonMatchSymbols,
  "\\P{sc=Vithkuqi}"
);
testPropertyEscapes(
  /^\P{sc=Vith}+$/u,
  nonMatchSymbols,
  "\\P{sc=Vith}"
);

reportCompare(0, 0);

// Copyright 2021 Mathias Bynens. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
author: Mathias Bynens
description: >
  Unicode property escapes for `General_Category=Number`
info: |
  Generated by https://github.com/mathiasbynens/unicode-property-escapes-tests
  Unicode v14.0.0
esid: sec-static-semantics-unicodematchproperty-p
features: [regexp-unicode-property-escapes]
includes: [regExpUtils.js]
---*/

const matchSymbols = buildString({
  loneCodePoints: [
    0x0000B9,
    0x002070,
    0x002CFD,
    0x003007,
    0x010341,
    0x01034A
  ],
  ranges: [
    [0x000030, 0x000039],
    [0x0000B2, 0x0000B3],
    [0x0000BC, 0x0000BE],
    [0x000660, 0x000669],
    [0x0006F0, 0x0006F9],
    [0x0007C0, 0x0007C9],
    [0x000966, 0x00096F],
    [0x0009E6, 0x0009EF],
    [0x0009F4, 0x0009F9],
    [0x000A66, 0x000A6F],
    [0x000AE6, 0x000AEF],
    [0x000B66, 0x000B6F],
    [0x000B72, 0x000B77],
    [0x000BE6, 0x000BF2],
    [0x000C66, 0x000C6F],
    [0x000C78, 0x000C7E],
    [0x000CE6, 0x000CEF],
    [0x000D58, 0x000D5E],
    [0x000D66, 0x000D78],
    [0x000DE6, 0x000DEF],
    [0x000E50, 0x000E59],
    [0x000ED0, 0x000ED9],
    [0x000F20, 0x000F33],
    [0x001040, 0x001049],
    [0x001090, 0x001099],
    [0x001369, 0x00137C],
    [0x0016EE, 0x0016F0],
    [0x0017E0, 0x0017E9],
    [0x0017F0, 0x0017F9],
    [0x001810, 0x001819],
    [0x001946, 0x00194F],
    [0x0019D0, 0x0019DA],
    [0x001A80, 0x001A89],
    [0x001A90, 0x001A99],
    [0x001B50, 0x001B59],
    [0x001BB0, 0x001BB9],
    [0x001C40, 0x001C49],
    [0x001C50, 0x001C59],
    [0x002074, 0x002079],
    [0x002080, 0x002089],
    [0x002150, 0x002182],
    [0x002185, 0x002189],
    [0x002460, 0x00249B],
    [0x0024EA, 0x0024FF],
    [0x002776, 0x002793],
    [0x003021, 0x003029],
    [0x003038, 0x00303A],
    [0x003192, 0x003195],
    [0x003220, 0x003229],
    [0x003248, 0x00324F],
    [0x003251, 0x00325F],
    [0x003280, 0x003289],
    [0x0032B1, 0x0032BF],
    [0x00A620, 0x00A629],
    [0x00A6E6, 0x00A6EF],
    [0x00A830, 0x00A835],
    [0x00A8D0, 0x00A8D9],
    [0x00A900, 0x00A909],
    [0x00A9D0, 0x00A9D9],
    [0x00A9F0, 0x00A9F9],
    [0x00AA50, 0x00AA59],
    [0x00ABF0, 0x00ABF9],
    [0x00FF10, 0x00FF19],
    [0x010107, 0x010133],
    [0x010140, 0x010178],
    [0x01018A, 0x01018B],
    [0x0102E1, 0x0102FB],
    [0x010320, 0x010323],
    [0x0103D1, 0x0103D5],
    [0x0104A0, 0x0104A9],
    [0x010858, 0x01085F],
    [0x010879, 0x01087F],
    [0x0108A7, 0x0108AF],
    [0x0108FB, 0x0108FF],
    [0x010916, 0x01091B],
    [0x0109BC, 0x0109BD],
    [0x0109C0, 0x0109CF],
    [0x0109D2, 0x0109FF],
    [0x010A40, 0x010A48],
    [0x010A7D, 0x010A7E],
    [0x010A9D, 0x010A9F],
    [0x010AEB, 0x010AEF],
    [0x010B58, 0x010B5F],
    [0x010B78, 0x010B7F],
    [0x010BA9, 0x010BAF],
    [0x010CFA, 0x010CFF],
    [0x010D30, 0x010D39],
    [0x010E60, 0x010E7E],
    [0x010F1D, 0x010F26],
    [0x010F51, 0x010F54],
    [0x010FC5, 0x010FCB],
    [0x011052, 0x01106F],
    [0x0110F0, 0x0110F9],
    [0x011136, 0x01113F],
    [0x0111D0, 0x0111D9],
    [0x0111E1, 0x0111F4],
    [0x0112F0, 0x0112F9],
    [0x011450, 0x011459],
    [0x0114D0, 0x0114D9],
    [0x011650, 0x011659],
    [0x0116C0, 0x0116C9],
    [0x011730, 0x01173B],
    [0x0118E0, 0x0118F2],
    [0x011950, 0x011959],
    [0x011C50, 0x011C6C],
    [0x011D50, 0x011D59],
    [0x011DA0, 0x011DA9],
    [0x011FC0, 0x011FD4],
    [0x012400, 0x01246E],
    [0x016A60, 0x016A69],
    [0x016AC0, 0x016AC9],
    [0x016B50, 0x016B59],
    [0x016B5B, 0x016B61],
    [0x016E80, 0x016E96],
    [0x01D2E0, 0x01D2F3],
    [0x01D360, 0x01D378],
    [0x01D7CE, 0x01D7FF],
    [0x01E140, 0x01E149],
    [0x01E2F0, 0x01E2F9],
    [0x01E8C7, 0x01E8CF],
    [0x01E950, 0x01E959],
    [0x01EC71, 0x01ECAB],
    [0x01ECAD, 0x01ECAF],
    [0x01ECB1, 0x01ECB4],
    [0x01ED01, 0x01ED2D],
    [0x01ED2F, 0x01ED3D],
    [0x01F100, 0x01F10C],
    [0x01FBF0, 0x01FBF9]
  ]
});
testPropertyEscapes(
  /^\p{General_Category=Number}+$/u,
  matchSymbols,
  "\\p{General_Category=Number}"
);
testPropertyEscapes(
  /^\p{General_Category=N}+$/u,
  matchSymbols,
  "\\p{General_Category=N}"
);
testPropertyEscapes(
  /^\p{gc=Number}+$/u,
  matchSymbols,
  "\\p{gc=Number}"
);
testPropertyEscapes(
  /^\p{gc=N}+$/u,
  matchSymbols,
  "\\p{gc=N}"
);
testPropertyEscapes(
  /^\p{Number}+$/u,
  matchSymbols,
  "\\p{Number}"
);
testPropertyEscapes(
  /^\p{N}+$/u,
  matchSymbols,
  "\\p{N}"
);

const nonMatchSymbols = buildString({
  loneCodePoints: [
    0x003250,
    0x016B5A,
    0x01ECAC,
    0x01ECB0,
    0x01ED2E
  ],
  ranges: [
    [0x00DC00, 0x00DFFF],
    [0x000000, 0x00002F],
    [0x00003A, 0x0000B1],
    [0x0000B4, 0x0000B8],
    [0x0000BA, 0x0000BB],
    [0x0000BF, 0x00065F],
    [0x00066A, 0x0006EF],
    [0x0006FA, 0x0007BF],
    [0x0007CA, 0x000965],
    [0x000970, 0x0009E5],
    [0x0009F0, 0x0009F3],
    [0x0009FA, 0x000A65],
    [0x000A70, 0x000AE5],
    [0x000AF0, 0x000B65],
    [0x000B70, 0x000B71],
    [0x000B78, 0x000BE5],
    [0x000BF3, 0x000C65],
    [0x000C70, 0x000C77],
    [0x000C7F, 0x000CE5],
    [0x000CF0, 0x000D57],
    [0x000D5F, 0x000D65],
    [0x000D79, 0x000DE5],
    [0x000DF0, 0x000E4F],
    [0x000E5A, 0x000ECF],
    [0x000EDA, 0x000F1F],
    [0x000F34, 0x00103F],
    [0x00104A, 0x00108F],
    [0x00109A, 0x001368],
    [0x00137D, 0x0016ED],
    [0x0016F1, 0x0017DF],
    [0x0017EA, 0x0017EF],
    [0x0017FA, 0x00180F],
    [0x00181A, 0x001945],
    [0x001950, 0x0019CF],
    [0x0019DB, 0x001A7F],
    [0x001A8A, 0x001A8F],
    [0x001A9A, 0x001B4F],
    [0x001B5A, 0x001BAF],
    [0x001BBA, 0x001C3F],
    [0x001C4A, 0x001C4F],
    [0x001C5A, 0x00206F],
    [0x002071, 0x002073],
    [0x00207A, 0x00207F],
    [0x00208A, 0x00214F],
    [0x002183, 0x002184],
    [0x00218A, 0x00245F],
    [0x00249C, 0x0024E9],
    [0x002500, 0x002775],
    [0x002794, 0x002CFC],
    [0x002CFE, 0x003006],
    [0x003008, 0x003020],
    [0x00302A, 0x003037],
    [0x00303B, 0x003191],
    [0x003196, 0x00321F],
    [0x00322A, 0x003247],
    [0x003260, 0x00327F],
    [0x00328A, 0x0032B0],
    [0x0032C0, 0x00A61F],
    [0x00A62A, 0x00A6E5],
    [0x00A6F0, 0x00A82F],
    [0x00A836, 0x00A8CF],
    [0x00A8DA, 0x00A8FF],
    [0x00A90A, 0x00A9CF],
    [0x00A9DA, 0x00A9EF],
    [0x00A9FA, 0x00AA4F],
    [0x00AA5A, 0x00ABEF],
    [0x00ABFA, 0x00DBFF],
    [0x00E000, 0x00FF0F],
    [0x00FF1A, 0x010106],
    [0x010134, 0x01013F],
    [0x010179, 0x010189],
    [0x01018C, 0x0102E0],
    [0x0102FC, 0x01031F],
    [0x010324, 0x010340],
    [0x010342, 0x010349],
    [0x01034B, 0x0103D0],
    [0x0103D6, 0x01049F],
    [0x0104AA, 0x010857],
    [0x010860, 0x010878],
    [0x010880, 0x0108A6],
    [0x0108B0, 0x0108FA],
    [0x010900, 0x010915],
    [0x01091C, 0x0109BB],
    [0x0109BE, 0x0109BF],
    [0x0109D0, 0x0109D1],
    [0x010A00, 0x010A3F],
    [0x010A49, 0x010A7C],
    [0x010A7F, 0x010A9C],
    [0x010AA0, 0x010AEA],
    [0x010AF0, 0x010B57],
    [0x010B60, 0x010B77],
    [0x010B80, 0x010BA8],
    [0x010BB0, 0x010CF9],
    [0x010D00, 0x010D2F],
    [0x010D3A, 0x010E5F],
    [0x010E7F, 0x010F1C],
    [0x010F27, 0x010F50],
    [0x010F55, 0x010FC4],
    [0x010FCC, 0x011051],
    [0x011070, 0x0110EF],
    [0x0110FA, 0x011135],
    [0x011140, 0x0111CF],
    [0x0111DA, 0x0111E0],
    [0x0111F5, 0x0112EF],
    [0x0112FA, 0x01144F],
    [0x01145A, 0x0114CF],
    [0x0114DA, 0x01164F],
    [0x01165A, 0x0116BF],
    [0x0116CA, 0x01172F],
    [0x01173C, 0x0118DF],
    [0x0118F3, 0x01194F],
    [0x01195A, 0x011C4F],
    [0x011C6D, 0x011D4F],
    [0x011D5A, 0x011D9F],
    [0x011DAA, 0x011FBF],
    [0x011FD5, 0x0123FF],
    [0x01246F, 0x016A5F],
    [0x016A6A, 0x016ABF],
    [0x016ACA, 0x016B4F],
    [0x016B62, 0x016E7F],
    [0x016E97, 0x01D2DF],
    [0x01D2F4, 0x01D35F],
    [0x01D379, 0x01D7CD],
    [0x01D800, 0x01E13F],
    [0x01E14A, 0x01E2EF],
    [0x01E2FA, 0x01E8C6],
    [0x01E8D0, 0x01E94F],
    [0x01E95A, 0x01EC70],
    [0x01ECB5, 0x01ED00],
    [0x01ED3E, 0x01F0FF],
    [0x01F10D, 0x01FBEF],
    [0x01FBFA, 0x10FFFF]
  ]
});
testPropertyEscapes(
  /^\P{General_Category=Number}+$/u,
  nonMatchSymbols,
  "\\P{General_Category=Number}"
);
testPropertyEscapes(
  /^\P{General_Category=N}+$/u,
  nonMatchSymbols,
  "\\P{General_Category=N}"
);
testPropertyEscapes(
  /^\P{gc=Number}+$/u,
  nonMatchSymbols,
  "\\P{gc=Number}"
);
testPropertyEscapes(
  /^\P{gc=N}+$/u,
  nonMatchSymbols,
  "\\P{gc=N}"
);
testPropertyEscapes(
  /^\P{Number}+$/u,
  nonMatchSymbols,
  "\\P{Number}"
);
testPropertyEscapes(
  /^\P{N}+$/u,
  nonMatchSymbols,
  "\\P{N}"
);

reportCompare(0, 0);

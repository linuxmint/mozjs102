// |reftest| skip -- Temporal is not supported
// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.prototype.getoffsetstringfor
description: Negative zero, as an extended year, is rejected
features: [Temporal, arrow-function]
---*/

const invalidStrings = [
  "-000000-03-30T00:45Z",
  "-000000-03-30T01:45+01:00",
  "-000000-03-30T01:45:00+00:00[UTC]",
];
const instance = new Temporal.TimeZone("UTC");
invalidStrings.forEach((arg) => {
  assert.throws(
    RangeError,
    () => instance.getOffsetStringFor(arg),
    "reject minus zero as extended year"
  );
});

reportCompare(0, 0);

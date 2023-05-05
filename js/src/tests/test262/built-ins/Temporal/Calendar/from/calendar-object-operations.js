// |reftest| skip -- Temporal is not supported
// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.from
description: Converting objects to Temporal.Calendar
includes: [compareArray.js, temporalHelpers.js]
features: [Temporal]
---*/

const expected = [
  "has outer.calendar",
  "get outer.calendar",
  "has inner.calendar",
  "get inner.toString",
  "call inner.toString",
];
const actual = [];
const calendar = new Proxy({}, {
  has(t, p) {
    actual.push(`has outer.${p}`);
    return true;
  },
  get(t, p) {
    actual.push(`get outer.${p}`);
    return new Proxy(TemporalHelpers.toPrimitiveObserver(actual, "iso8601", "inner"), {
      has(t, p) {
        actual.push(`has inner.${p}`);
        return true;
      },
      get(t, p) {
        return t[p];
      },
    });
  },
});
const result = Temporal.Calendar.from(calendar);
assert.sameValue(result.id, "iso8601");
assert.compareArray(actual, expected);

reportCompare(0, 0);

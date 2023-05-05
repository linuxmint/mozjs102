// |reftest| skip -- Temporal is not supported
// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.dayofyear
description: >
  Temporal.Calendar.prototype.dayOfYear does not implement [[Construct]], is not new-able
info: |
    Built-in function objects that are not identified as constructors do not implement the
    [[Construct]] internal method unless otherwise specified in the description of a particular
    function.
includes: [isConstructor.js]
features: [Reflect.construct, Temporal]
---*/

assert.throws(TypeError, () => {
  new Temporal.Calendar.prototype.dayOfYear();
}, "Calling as constructor");

assert.sameValue(isConstructor(Temporal.Calendar.prototype.dayOfYear), false,
  "isConstructor(Temporal.Calendar.prototype.dayOfYear)");

reportCompare(0, 0);

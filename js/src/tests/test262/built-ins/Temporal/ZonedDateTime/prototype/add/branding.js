// |reftest| skip -- Temporal is not supported
// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.add
description: Throw a TypeError if the receiver is invalid
features: [Symbol, Temporal]
---*/

const add = Temporal.ZonedDateTime.prototype.add;

assert.sameValue(typeof add, "function");

assert.throws(TypeError, () => add.call(undefined), "undefined");
assert.throws(TypeError, () => add.call(null), "null");
assert.throws(TypeError, () => add.call(true), "true");
assert.throws(TypeError, () => add.call(""), "empty string");
assert.throws(TypeError, () => add.call(Symbol()), "symbol");
assert.throws(TypeError, () => add.call(1), "1");
assert.throws(TypeError, () => add.call({}), "plain object");
assert.throws(TypeError, () => add.call(Temporal.ZonedDateTime), "Temporal.ZonedDateTime");
assert.throws(TypeError, () => add.call(Temporal.ZonedDateTime.prototype), "Temporal.ZonedDateTime.prototype");

reportCompare(0, 0);

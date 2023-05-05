// |reftest| skip -- ShadowRealm is not supported
// Copyright (C) 2021 Rick Waldron. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
esid: sec-shadowrealm.prototype.importvalue
description: >
  ShadowRealm.prototype.importValue is an ECMAScript Standard built-in object function.
includes: [propertyHelper.js]
features: [ShadowRealm]
---*/

verifyProperty(ShadowRealm.prototype, "importValue", {
  enumerable: false,
  writable: true,
  configurable: true,
});

reportCompare(0, 0);

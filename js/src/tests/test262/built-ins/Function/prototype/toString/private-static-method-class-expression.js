// Copyright (C) 2019 Kubilay Kahveci (Bloomberg LP). All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
description: Function.prototype.toString on a static private method
features: [class-static-methods-private]
includes: [nativeFunctionMatcher.js]
---*/

let C = class {
  /* before */static #f /* a */ ( /* b */ ) /* c */ { /* d */ }/* after */
  static assert(expected) {
    assertToStringOrNativeFunction(this.#f, expected);
  }
};

C.assert("#f /* a */ ( /* b */ ) /* c */ { /* d */ }");

reportCompare(0, 0);

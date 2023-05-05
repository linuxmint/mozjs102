// Copyright (C) 2021 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-ecmascript-function-objects-construct-argumentslist-newtarget
description: >
  TypeError from `return 0` is not catchable.
---*/

class C extends class {} {
  constructor() {
    super();

    try {
      return 0;
    } catch(e) {
      return;
    }
  }
}

assert.throws(TypeError, function() {
  new C();
});

reportCompare(0, 0);

// Copyright 2022 Mathias Bynens. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
author: Mathias Bynens
esid: prod-PrivateIdentifier
description: |
  Test that Unicode v10.0.0 ID_Continue characters are accepted as
  identifier part characters in escaped form, i.e.
  - \uXXXX or \u{XXXX} for BMP symbols
  - \u{XXXXXX} for astral symbols
  in private class fields.
info: |
  Generated by https://github.com/mathiasbynens/caniunicode
features: [class, class-fields-private]
---*/

class _ {
  #_\u0AFA\u0AFB\u0AFC\u0AFD\u0AFE\u0AFF\u0D00\u0D3B\u0D3C\u1CF7\u1DF6\u1DF7\u1DF8\u1DF9\u{11A01}\u{11A02}\u{11A03}\u{11A04}\u{11A05}\u{11A06}\u{11A07}\u{11A08}\u{11A09}\u{11A0A}\u{11A33}\u{11A34}\u{11A35}\u{11A36}\u{11A37}\u{11A38}\u{11A39}\u{11A3B}\u{11A3C}\u{11A3D}\u{11A3E}\u{11A47}\u{11A51}\u{11A52}\u{11A53}\u{11A54}\u{11A55}\u{11A56}\u{11A57}\u{11A58}\u{11A59}\u{11A5A}\u{11A5B}\u{11A8A}\u{11A8B}\u{11A8C}\u{11A8D}\u{11A8E}\u{11A8F}\u{11A90}\u{11A91}\u{11A92}\u{11A93}\u{11A94}\u{11A95}\u{11A96}\u{11A97}\u{11A98}\u{11A99}\u{11D31}\u{11D32}\u{11D33}\u{11D34}\u{11D35}\u{11D36}\u{11D3A}\u{11D3C}\u{11D3D}\u{11D3F}\u{11D40}\u{11D41}\u{11D42}\u{11D43}\u{11D44}\u{11D45}\u{11D47}\u{11D50}\u{11D51}\u{11D52}\u{11D53}\u{11D54}\u{11D55}\u{11D56}\u{11D57}\u{11D58}\u{11D59};
};

reportCompare(0, 0);

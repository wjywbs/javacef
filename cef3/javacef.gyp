# Copyright (c) 2014 javacef Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'includes': [
    'javacef-common.gypi',
  ],
  'targets': [
    {
      'target_name': 'javacef',
      'type': 'none',
      'msvs_guid': '00A7641D-D809-43E4-83D6-9DA153A126F1',
      'dependencies': [
        'chromium_loader/chromium_loader.gyp:chromium_loader',
      ],
    },
  ],
}

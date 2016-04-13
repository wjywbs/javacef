# Copyright (c) 2014 javacef Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
    'framework_name': 'Chromium Embedded Framework',
    'linux_use_gold_binary': 0,
    'linux_use_gold_flags': 0,
    # Don't use clang plugins to avoid plugin check.
    'clang_use_chrome_plugins': 0,
    'conditions': [
      [ 'OS=="mac"', {
        # Don't use clang with CEF binary releases due to Chromium tree structure dependency.
        'clang': 0,
        'target_arch': 'x64',
      }],
      ['sysroot!=""', {
        'pkg-config': './pkg-config-wrapper "<(sysroot)" "<(target_arch)"',
      }, {
        'pkg-config': 'pkg-config'
      }],
      [ 'OS=="win"', {
        'multi_threaded_dll%': 0,
      }],
    ]
  }
}

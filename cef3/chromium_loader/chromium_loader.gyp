# Copyright (c) 2014 javacef Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'includes': [
    '../javacef-common.gypi',
    'cef_paths2.gypi',
  ],
  'targets': [
    {
      'target_name': 'chromium_loader',
      'type': 'shared_library',
      'msvs_guid': '03FD7FF3-4ACB-4ABC-B8E6-CE28FBB54A12',
      'dependencies': [
        'libcef_dll_wrapper',
        '../java/java_sources.gyp:build_java',
      ],
      'defines': [
        'USING_CEF_SHARED',
      ],
      'include_dirs': [
        '.',
        '<(java_include_path)',
      ],
      'sources': [
        '<@(includes_common)',
        '<@(includes_wrapper)',
        '<@(chromium_loader_sources_common)',
      ],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-Wno-c++11-extensions',
          '-Wno-unused-function',
          '-Wno-unused-variable',
          '-Wno-newline-eof',
          '-Wno-deprecated-declarations',
        ],
      },
      'conditions': [
        ['OS=="win"', {
          'variables': {
            'msvs_cef_lib_path': 'lib/libcef-$(PlatformName)-$(ConfigurationName).lib',
            'cef_lib_32_path': 'lib/libcef-win32-Release.lib',
            'cef_lib_64_path': 'lib/libcef-x64-Release.lib',
            'gyp_generators': '<!(echo %GYP_GENERATORS%)',
          },
          'include_dirs': [
            '<(java_include_path)/win32',
            '<(SHARED_INTERMEDIATE_DIR)',
          ],
          'msvs_disabled_warnings': [4800],
          'msvs_settings': {
            'VCLinkerTool': {
              # Set /SUBSYSTEM:WINDOWS.
              'SubSystem': '2',
            },
          },
          'link_settings': {
            'libraries': [
              '-lcomctl32.lib',
              '-lshlwapi.lib',
              '-lrpcrt4.lib',
              '-lopengl32.lib',
              '-lglu32.lib',
            ],
          },
          'library_dirs': [
            # Needed to find cef_sandbox.lib using #pragma comment(lib, ...).
            'lib',
          ],
          'conditions': [
            ['gyp_generators=="ninja"', {
              # TODO: choose build type
              'conditions': [
                ['target_arch=="x64"', {
                  'link_settings': {
                    'libraries': [
                      '-l../../chromium_loader/<(cef_lib_64_path)',
                    ],
                  },
                }, {
                  'link_settings': {
                    'libraries': [
                      '-l../../chromium_loader/<(cef_lib_32_path)',
                    ],
                  },
                }],
              ],
            }, {
              'link_settings': {
                'libraries': [
                  '-l<(msvs_cef_lib_path)',
                ],
              },
            }],
          ],
          'sources': [
            '<@(includes_win)',
            '<@(cefclient_sources_win)',
            '<@(chromium_loader_sources_win)',
          ],
        }],
        [ 'OS=="mac"', {
          'include_dirs': [
            '<(java_include_path)/darwin',
            '<(SHARED_INTERMEDIATE_DIR)',
          ],
          'link_settings': {
            'libraries': [
              '<!(echo $SDKROOT)/System/Library/Frameworks/AppKit.framework',
              '<!(echo $SDKROOT)/System/Library/Frameworks/OpenGL.framework',
              '<(cef_prebuilt_path)/Contents/Frameworks/Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework',
            ],
          },
          'sources': [
            '<@(includes_mac)',
            '<@(cefclient_sources_mac)',
            '<@(chromium_loader_sources_mac)',
          ],
        }],
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'dependencies': [
            'gtk',
            'gtkglext',
          ],
          'include_dirs': [
            '<(java_include_path)/linux',
            '<(SHARED_INTERMEDIATE_DIR)',
          ],
          'link_settings': {
            'ldflags': [
              # Look for libcef.so in the current directory. Path can also be
              # specified using the LD_LIBRARY_PATH environment variable.  -fvisibility=default
              '-Wl,-rpath,.',
            ],
            'libraries': [
              "<(cef_prebuilt_path)/lib/libcef.so",
            ],
          },
          'sources': [
            '<@(includes_linux)',
            '<@(cefclient_sources_linux)',
            '<@(chromium_loader_sources_linux)',
          ],
        }],
      ],
    },
    {
      'target_name': 'libcef_dll_wrapper',
      'type': 'static_library',
      'msvs_guid': 'A9D6DC71-C0DC-4549-AEA0-3B15B44E86A9',
      'defines': [
        'USING_CEF_SHARED',
      ],
      'include_dirs': [
        '.',
      ],
      'sources': [
        '<@(includes_common)',
        '<@(includes_capi)',
        '<@(includes_wrapper)',
        '<@(libcef_dll_wrapper_sources_common)',
      ],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-Wno-c++11-extensions',
        ],
      },
      'conditions': [
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'dependencies': [
            'gtk',
          ],
        }],
        [ 'OS=="win" and multi_threaded_dll', {
          'configurations': {
            'Debug': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'RuntimeLibrary': 3,
                  'WarnAsError': 'false',
                },
              },
            },
            'Release': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'RuntimeLibrary': 2,
                  'WarnAsError': 'false',
                },
              },
            }
          }
        }],
      ],
    },
  ],
  'conditions': [
    [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
      'targets': [
        {
          'target_name': 'gtk',
          'type': 'none',
          'variables': {
            # gtk requires gmodule, but it does not list it as a dependency
            # in some misconfigured systems.
            'gtk_packages': 'gmodule-2.0 gtk+-2.0 gthread-2.0 gtk+-unix-print-2.0',
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!(<(pkg-config) --cflags <(gtk_packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!(<(pkg-config) --libs-only-L --libs-only-other <(gtk_packages))',
            ],
            'libraries': [
              '<!(<(pkg-config) --libs-only-l <(gtk_packages))',
            ],
          },
        },
        {
          'target_name': 'gtkglext',
          'type': 'none',
          'variables': {
            # gtkglext is required by the cefclient OSR example.
            'gtk_packages': 'gtkglext-1.0',
          },
          'direct_dependent_settings': {
            'cflags': [
              '<!(<(pkg-config) --cflags <(gtk_packages))',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!(<(pkg-config) --libs-only-L --libs-only-other <(gtk_packages))',
            ],
            'libraries': [
              '<!(<(pkg-config) --libs-only-l <(gtk_packages))',
            ],
          },
        },
      ],
    }],  # OS=="linux" or OS=="freebsd" or OS=="openbsd"
  ],
}

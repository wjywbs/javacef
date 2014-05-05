# Copyright (c) 2014 javacef Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'includes': [
    '../javacef-common.gypi',
  ],
  'variables': {
    'java_sources': [
      'src/org/embedded/browser/ChromeSettings.java',
      'src/org/embedded/browser/ChromeWindow.java',
      'src/org/embedded/browser/Chromium.java',
      'src/org/embedded/browser/CloseWindowListener.java',
      'src/org/embedded/browser/DownloadWindow.java',
      'src/org/embedded/browser/LoadStatusListener.java',
      'src/org/embedded/browser/NavStateListener.java',
      'src/org/embedded/browser/NewWindowListener.java',
      'src/org/embedded/browser/SampleBrowserAWT.java',
      'src/org/embedded/browser/SampleBrowserSWT.java',
      'src/org/embedded/browser/SampleWindow.java',
      'src/org/embedded/browser/TitleChangeListener.java',
    ],
    'java_resources': [
      'resources/arrow_left.png',
      'resources/arrow_refresh.png',
      'resources/arrow_right.png',
      'resources/page-loader.gif',
    ],
    'java_home': '<!(echo $JAVA_HOME)',
    'path_delimiter': ':',
  },
  'conditions': [
    ['OS=="win"', {
      'variables': {
        'java_home': '"<!(echo %JAVA_HOME%)"',
        'path_delimiter': ';',
      },
    }],
  ],
  'targets': [
    {
      'target_name': 'build_java_sources',
      'type': 'none',
      'msvs_guid': '6D0C3577-815C-4A03-ACA5-0E5F3116C8CA',
      'conditions': [
        [ 'OS=="win" or OS=="mac" or OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'conditions': [
            ['target_arch=="x64"', {
              'variables': {
                'swt_library_path': 'lib/<(OS)/swt64.jar',
              },
            }, {
              'variables': {
                'swt_library_path': 'lib/<(OS)/swt32.jar',
              },
            }],
          ],
          'actions': [
            {
              'action_name': 'build_java_files',
              'inputs': [
                '<@(java_sources)',
              ],
              'outputs': [
                '<(PRODUCT_DIR)/bin/org/embedded/browser/ChromeSettings.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/ChromeWindow.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/Chromium.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/CloseWindowListener.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/DownloadWindow.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/LoadStatusListener.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/NavStateListener.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/NewWindowListener.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/SampleBrowserAWT.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/SampleBrowserSWT.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/SampleWindow.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/TitleChangeListener.class',
              ],
              'action': ['<(java_home)/bin/javac', '-cp', '<(swt_library_path)', '-d', '<(PRODUCT_DIR)/bin', '<@(java_sources)'],
            },
            {
              'variables': {
                'java_classes': [
                  'org.embedded.browser.Chromium',
                  'org.embedded.browser.DownloadWindow',
                ],
              },
              'action_name': 'generate_java_jni_headers',
              'inputs': [
                '<(PRODUCT_DIR)/bin/org/embedded/browser/ChromeSettings.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/Chromium.class',
                '<(PRODUCT_DIR)/bin/org/embedded/browser/DownloadWindow.class',
              ],
              'outputs': [
                '<(SHARED_INTERMEDIATE_DIR)/org_embedded_browser_Chromium.h',
                '<(SHARED_INTERMEDIATE_DIR)/org_embedded_browser_DownloadWindow.h',
              ],
              'action': ['<(java_home)/bin/javah', '-classpath', '<(swt_library_path)<(path_delimiter)<(PRODUCT_DIR)/bin', '-d', '<(SHARED_INTERMEDIATE_DIR)', '<@(java_classes)'],
            },
          ],
        }],
      ],
    },
    {
      'target_name': 'build_java',
      'type': 'none',
      'msvs_guid': '1748776D-CAC1-4B6B-9119-97EA48EF93B8',
      'dependencies': [
        'build_java_sources',
      ],
      'actions': [
        {
          'action_name': 'pack_jar',
          'conditions': [
            ['target_arch=="x64"', {
              'variables': {
                'jar_path': '<(PRODUCT_DIR)/javacef3_<(OS)64.jar',
              },
            }, {
              'variables': {
                'jar_path': '<(PRODUCT_DIR)/javacef3_<(OS)32.jar',
              },
            }],
          ],
          'inputs': [
            '<@(java_sources)',
          ],
          'outputs': [
            '<(jar_path)',
          ],
          'action': ['<(java_home)/bin/jar', 'cf', '<(jar_path)', '-C', '<(PRODUCT_DIR)/bin', '.'],
        },
      ],
    },
  ],
}

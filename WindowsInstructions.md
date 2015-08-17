## Introduction ##

This wiki page has instructions about how to use or build in Windows.

## Use ##

Currently:
  * Download all the sources using git command line. You can follow the steps on the [Source](http://code.google.com/p/javacef/source/checkout) tab.
  * Set an environment variable "jdk" to jdk\_path\bin. For me, it's D:\Program Files (x86)\Java\jdk1.7.0\_21\bin
  * Under command prompt, cd to windows\java. Run:
```
"%jdk%\java" -cp bin\javacef1-win32.jar;bin\swt.jar -Djava.library.path=..\cef_runtime org.embedded.browser.SampleBrowserAWT
```

## Build from source ##

  * Download all the sources using git command line. You can follow the steps on the [Source](http://code.google.com/p/javacef/source/checkout) tab.
  * Set an environment variable "jdk" to jdk\_path\bin. For me, it's D:\Program Files (x86)\Java\jdk1.7.0\_21\bin
  * Copy all files except chromium\_loader\_win.dll in cef\_runtime folder to jdk\_path\bin.
  * Make a backup to all files under windows\java\bin. This folder will be cleaned by Eclipse.
  * Open Eclipse and import existing project under windows\java.
  * Eclipse will normally build the project.
  * Copy buildhead.bat and swt.jar back to windows\java\bin.
  * Run buildhead.bat to generate jni headers. You may need to decompress swt.jar.
  * Open windows\chromium\_loader\_win\chromium\_loader\_win.sln and build it.
  * Copy compiled chromium\_loader\_win.dll to jdk\_path\bin.
  * You can run SampleBrowserAWT under Eclipse. You may need to add swt.jar to path in Eclipse.